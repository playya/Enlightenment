#include "Ecore.h"
#include "config.h"
#include "Ecore_Con.h"
#include "ecore_private.h"
#include "ecore_ipc_private.h"
#include "Ecore_Ipc.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#elif WIN32
#include <winsock.h>
#endif

#define DLT_ZERO   0
#define DLT_ONE    1
#define DLT_SAME   2
#define DLT_SHL    3
#define DLT_SHR    4
#define DLT_ADD8   5
#define DLT_DEL8   6
#define DLT_ADDU8  7
#define DLT_DELU8  8
#define DLT_ADD16  9
#define DLT_DEL16  10
#define DLT_ADDU16 11
#define DLT_DELU16 12
#define DLT_SET    13
#define DLT_R1     14
#define DLT_R2     15

static int _ecore_ipc_dlt_int(int out, int prev, int *mode);
static int _ecore_ipc_ddlt_int(int in, int prev, int mode);

static int
_ecore_ipc_dlt_int(int out, int prev, int *mode)
{
   int dlt;

   /* 0 byte */
   if (out == 0)
     {
	*mode = DLT_ZERO;
	return 0;
     }
   if (out == (int)0xffffffff)
     {
	*mode = DLT_ONE;
	return 0;
     }
   if (out == prev)
     {
	*mode = DLT_SAME;
	return 0;
     }
   if (out == prev << 1)
     {
	*mode = DLT_SHL;
	return 0;
     }
   if (out == prev >> 1)
     {
	*mode = DLT_SHR;
	return 0;
     }
   /* 1 byte */
   dlt = out - prev;
   if (!(dlt & 0xffffff00))
     {
	*mode = DLT_ADD8;
	return dlt & 0xff;
     }
   dlt = prev - out;
   if (!(dlt & 0xffffff00))
     {
	*mode = DLT_DEL8;
	return dlt & 0xff;
     }
   dlt = out - prev;
   if (!(dlt & 0x00ffffff))
     {
	*mode = DLT_ADDU8;
	return (dlt >> 24) & 0xff;
     }
   dlt = prev - out;
   if (!(dlt & 0x00ffffff))
     {
	*mode = DLT_DELU8;
	return (dlt >> 24) & 0xff;
     }
   /* 2 byte */
   dlt = out - prev;
   if (!(dlt & 0xffff0000))
     {
	*mode = DLT_ADD16;
	return dlt & 0xfffff;
     }
   dlt = prev - out;
   if (!(dlt & 0xffff0000))
     {
	*mode = DLT_DEL16;
	return dlt & 0xffff;
     }
   dlt = out - prev;
   if (!(dlt & 0x0000ffff))
     {
	*mode = DLT_ADDU16;
	return (dlt >> 16) & 0xffff;
     }
   dlt = prev - out;
   if (!(dlt & 0x0000ffff))
     {
	*mode = DLT_DELU16;
	return (dlt >> 16) & 0xffff;
     }
   /* 4 byte */
   *mode = DLT_SET;
   return out;
}

static int
_ecore_ipc_ddlt_int(int in, int prev, int mode)
{
   switch (mode)
     {
      case DLT_ZERO:
	return 0;
	break;
      case DLT_ONE:
	return 0xffffffff;
	break;
      case DLT_SAME:
	return prev;
	break;
      case DLT_SHL:
	return prev << 1;
	break;
      case DLT_SHR:
	return prev >> 1;
	break;
      case DLT_ADD8:
	return prev + in;
	break;
      case DLT_DEL8:
	return prev - in;
	break;
      case DLT_ADDU8:
	return prev + (in << 24);
	break;
      case DLT_DELU8:
	return prev - (in << 24);
	break;
      case DLT_ADD16:
	return prev + in;
	break;
      case DLT_DEL16:
	return prev - in;
	break;
      case DLT_ADDU16:
	return prev + (in << 16);
	break;
      case DLT_DELU16:
	return prev - (in << 16);
	break;
      case DLT_SET:
	return in;
	break;
      case DLT_R1:
	return 0;
	break;
      case DLT_R2:
	return 0;
	break;
      default:
	break;
     }
   return 0;
}

static int _ecore_ipc_event_client_add(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_client_del(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_server_add(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_server_del(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_client_data(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_server_data(void *data, int ev_type, void *ev);
static void _ecore_ipc_event_client_data_free(void *data, void *ev);
static void _ecore_ipc_event_server_data_free(void *data, void *ev);
    
int ECORE_IPC_EVENT_CLIENT_ADD = 0;
int ECORE_IPC_EVENT_CLIENT_DEL = 0;
int ECORE_IPC_EVENT_SERVER_ADD = 0;
int ECORE_IPC_EVENT_SERVER_DEL = 0;
int ECORE_IPC_EVENT_CLIENT_DATA = 0;
int ECORE_IPC_EVENT_SERVER_DATA = 0;

static int init_count = 0;
static Ecore_Ipc_Server *servers = NULL;

/**
 * @defgroup Ecore_IPC_Library_Group IPC Library Functions
 *
 * Functions that set up and shut down the Ecore IPC Library.
 */

/**
 * Initialises the Ecore IPC library.
 * @return  Number of times the library has been initialised without
 *          being shut down.
 * @ingroup Ecore_IPC_Library_Group
 */
int
ecore_ipc_init(void)
{
   if (!init_count) ecore_con_init();
   init_count++;
   if (!ECORE_IPC_EVENT_CLIENT_ADD)
     {
	ECORE_IPC_EVENT_CLIENT_ADD = ecore_event_type_new();
	ECORE_IPC_EVENT_CLIENT_DEL = ecore_event_type_new();
	ECORE_IPC_EVENT_SERVER_ADD = ecore_event_type_new();
	ECORE_IPC_EVENT_SERVER_DEL = ecore_event_type_new();
	ECORE_IPC_EVENT_CLIENT_DATA = ecore_event_type_new();
	ECORE_IPC_EVENT_SERVER_DATA = ecore_event_type_new();
	
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _ecore_ipc_event_client_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _ecore_ipc_event_client_del, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, _ecore_ipc_event_server_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, _ecore_ipc_event_server_del, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _ecore_ipc_event_client_data, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, _ecore_ipc_event_server_data, NULL);
     }
   return init_count;   
}

/**
 * Shuts down the Ecore IPC library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @ingroup Ecore_IPC_Library_Group
 */
int
ecore_ipc_shutdown(void)
{
   if (init_count > 0)
     {
	init_count--;
	if (init_count > 0) return init_count;
	while (servers) ecore_ipc_server_del(servers);
	ecore_con_shutdown();
     }
   return 0;
}

/**
 * @defgroup Ecore_IPC_Server_Group IPC Server Functions
 *
 * Functions the deal with IPC server objects.
 */

/**
 * Creates an IPC server that listens for connections.
 *
 * For more details about the @p compl_type, @p name and @p port
 * parameters, see the @ref ecore_con_server_add documentation.
 *
 * @param   compl_type The connection type.
 * @param   name       Name to associate with the socket used for connection.
 * @param   port       Number to identify with socket used for connection.
 * @param   data       Data to associate with the IPC server.
 * @return  New IPC server.  If there is an error, @c NULL is returned.
 * @ingroup Ecore_IPC_Server_Group
 * @todo    Need to add protocol type parameter to this function.
 */
Ecore_Ipc_Server *
ecore_ipc_server_add(Ecore_Ipc_Type compl_type, const char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   Ecore_Ipc_Type type;
   Ecore_Con_Type extra = 0;
   
   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   if (!svr) return NULL;
   type = compl_type;
   type &= ~ECORE_IPC_USE_SSL;
   if (compl_type & ECORE_IPC_USE_SSL) extra = ECORE_CON_USE_SSL;
   switch (type)
     {
      case ECORE_IPC_LOCAL_USER:
	svr->server = ecore_con_server_add(ECORE_CON_LOCAL_USER | extra, name, port, svr);
	break;
      case ECORE_IPC_LOCAL_SYSTEM:
	svr->server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM | extra, name, port, svr);
	break;
      case ECORE_IPC_REMOTE_SYSTEM:
	svr->server = ecore_con_server_add(ECORE_CON_REMOTE_SYSTEM | extra, name, port, svr);
	break;
      default:
	free(svr);
	return NULL;
     }
   if (!svr->server)
     {
	free(svr);
	return NULL;
     }
   svr->data = (void *)data;
   servers = _ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   return svr;
}

/**
 * Creates an IPC server object to represent the IPC server listening
 * on the given port.
 *
 * For more details about the @p compl_type, @p name and @p port
 * parameters, see the @ref ecore_con_server_connect documentation.
 *
 * @param   compl_type The IPC connection type.
 * @param   name       Name used to determine which socket to use for the
 *                     IPC connection.
 * @param   port       Number used to identify the socket to use for the
 *                     IPC connection.
 * @param   data       Data to associate with the server.
 * @return  A new IPC server.  @c NULL is returned on error.
 * @ingroup Ecore_IPC_Server_Group
 * @todo    Need to add protocol type parameter.
 */
Ecore_Ipc_Server *
ecore_ipc_server_connect(Ecore_Ipc_Type compl_type, char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   Ecore_Ipc_Type type;
   Ecore_Con_Type extra = 0;
   
   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   if (!svr) return NULL;
   type = compl_type;
   type &= ~ECORE_IPC_USE_SSL;
   if (compl_type & ECORE_IPC_USE_SSL) extra = ECORE_CON_USE_SSL;
   switch (type)
     {
      case ECORE_IPC_LOCAL_USER:
	svr->server = ecore_con_server_connect(ECORE_CON_LOCAL_USER | extra, name, port, svr);
	break;
      case ECORE_IPC_LOCAL_SYSTEM:
	svr->server = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM | extra, name, port, svr);
	break;
      case ECORE_IPC_REMOTE_SYSTEM:
	svr->server = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM | extra, name, port, svr);
	break;
      default:
	free(svr);
	return NULL;
     }
   if (!svr->server)
     {
	free(svr);
	return NULL;
     }
   svr->data = (void *)data;
   servers = _ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   return svr;
}

/**
 * Closes the connection and frees the given IPC server.
 * @param   svr The given IPC server.
 * @return  The data associated with the server when it was created.
 * @ingroup Ecore_IPC_Server_Group
 */
void *
ecore_ipc_server_del(Ecore_Ipc_Server *svr)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_del");
	return NULL;
     }
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);
   data = svr->data;
   while (svr->clients) ecore_ipc_client_del((Ecore_Ipc_Client *)svr->clients);
   ecore_con_server_del(svr->server);
   servers = _ecore_list_remove(servers, svr);
   if (svr->buf) free(svr->buf);
   free(svr);
   return data;
}

/**
 * Retrieves the data associated with the given IPC server.
 * @param   svr The given IPC server.
 * @return  The associated data.
 * @ingroup Ecore_IPC_Server_Group
 */
void *
ecore_ipc_server_data_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_data_get");
	return NULL;
     }
   return svr->data;
}

/**
 * Retrieves whether the given IPC server is currently connected.
 * @param   svr The given IPC server.
 * @return  @c 1 if the server is connected.  @c 0 otherwise.
 * @ingroup Ecore_IPC_Server_Group
 */
int
ecore_ipc_server_connected_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_connected_get");
	return 0;
     }
   return ecore_con_server_connected_get(svr->server);
}

#define SVENC(_member) \
   d = _ecore_ipc_dlt_int(msg._member, svr->prev.o._member, &md); \
   if (md >= DLT_SET) \
     { \
	unsigned int v; \
	unsigned char *dd; \
	dd = (unsigned char *)&v; \
	v = d; \
	v = htonl(v); \
	*(dat + s + 0) = dd[0]; \
	*(dat + s + 1) = dd[1]; \
	*(dat + s + 2) = dd[2]; \
	*(dat + s + 3) = dd[3]; \
	s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
	unsigned short v; \
	unsigned char *dd; \
	dd = (unsigned char *)&v; \
	v = d; \
	v = htons(v); \
	*(dat + s + 0) = dd[0]; \
	*(dat + s + 1) = dd[1]; \
	s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
	*(dat + s + 0) = (unsigned char)d; \
	s += 1; \
     }

/**
 * Sends a message to the given IPC server.
 *
 * The content of the parameters, excluding the @p svr paramter, is up to
 * the client.
 *
 * @param   svr      The given IPC server.
 * @param   major    Major opcode of the message.
 * @param   minor    Minor opcode of the message.
 * @param   ref      Message reference number.
 * @param   ref_to   Reference number of the message this message refers to.
 * @param   response Requires response.
 * @param   data     The data to send as part of the message.
 * @param   size     Length of the data, in bytes, to send.
 * @return  Number of bytes sent.  @c 0 is returned if there is an error.
 * @ingroup Ecore_IPC_Server_Group
 * @todo    This function needs to become an IPC message.
 * @todo Fix up the documentation: Make sure what ref_to and response are.
 */
int
ecore_ipc_server_send(Ecore_Ipc_Server *svr, int major, int minor, int ref, int ref_to, int response, void *data, int size)
{
   Ecore_Ipc_Msg_Head msg;
   int ret;
   int *head, md = 0, d, s;
   unsigned char dat[sizeof(Ecore_Ipc_Msg_Head)];
   
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_send");
	return 0;
     }
   if (size < 0) size = 0;
   msg.major    = major;
   msg.minor    = minor;
   msg.ref      = ref;
   msg.ref_to   = ref_to;
   msg.response = response;
   msg.size     = size;
   head = (int *)dat;
   s = 4;
   SVENC(major);
   *head = md;
   SVENC(minor);
   *head |= md << (4 * 1);
   SVENC(ref);
   *head |= md << (4 * 2);
   SVENC(ref_to);
   *head |= md << (4 * 3);
   SVENC(response);
   *head |= md << (4 * 4);
   SVENC(size);
   *head |= md << (4 * 5);
   *head = htonl(*head);
   svr->prev.o = msg;
   ret = ecore_con_server_send(svr->server, dat, s);
   if (size > 0) ret += ecore_con_server_send(svr->server, data, size);
   return ret;
}

#define CLENC(_member) \
   d = _ecore_ipc_dlt_int(msg._member, cl->prev.o._member, &md); \
   if (md >= DLT_SET) \
     { \
	unsigned int v; \
	unsigned char *dd; \
	dd = (unsigned char *)&v; \
	v = d; \
	v = htonl(v); \
	*(dat + s + 0) = dd[0]; \
	*(dat + s + 1) = dd[1]; \
	*(dat + s + 2) = dd[2]; \
	*(dat + s + 3) = dd[3]; \
	s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
	unsigned short v; \
	unsigned char *dd; \
	dd = (unsigned char *)&v; \
	v = d; \
	v = htons(v); \
	*(dat + s + 0) = dd[0]; \
	*(dat + s + 1) = dd[1]; \
	s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
	*(dat + s) = (unsigned char)d; \
	s += 1; \
     }

/**
 * @defgroup Ecore_IPC_Client_Group IPC Client Functions
 *
 * Functions that deal with IPC client objects.
 */

/**
 * Sends a message to the given IPC client.
 * @param   cl       The given IPC client.
 * @param   major    Major opcode of the message.
 * @param   minor    Minor opcode of the message.
 * @param   ref      Reference number of the message.
 * @param   ref_to   Reference number of the message this message refers to.
 * @param   response Requires response.
 * @param   data     The data to send as part of the message.
 * @param   size     Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is
 *          an error.
 * @ingroup Ecore_IPC_Client_Group
 * @todo    This function needs to become an IPC message.
 * @todo    Make sure ref_to and response parameters are described correctly.
 */
int
ecore_ipc_client_send(Ecore_Ipc_Client *cl, int major, int minor, int ref, int ref_to, int response, void *data, int size)
{
   Ecore_Ipc_Msg_Head msg;
   int ret;
   int *head, md = 0, d, s;
   unsigned char dat[sizeof(Ecore_Ipc_Msg_Head)];
   
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_send");
	return 0;
     }
   if (size < 0) size = 0;
   msg.major    = major;
   msg.minor    = minor;
   msg.ref      = ref;
   msg.ref_to   = ref_to;
   msg.response = response;
   msg.size     = size;
   head = (int *)dat;
   s = 4;
   CLENC(major);
   *head = md;
   CLENC(minor);
   *head |= md << (4 * 1);
   CLENC(ref);
   *head |= md << (4 * 2);
   CLENC(ref_to);
   *head |= md << (4 * 3);
   CLENC(response);
   *head |= md << (4 * 4);
   CLENC(size);
   *head |= md << (4 * 5);
   *head = htonl(*head);
   cl->prev.o = msg;
   ret = ecore_con_client_send(cl->client, dat, s);
   if (size > 0) ret += ecore_con_client_send(cl->client, data, size);
   return ret;
}

/**
 * Retrieves the IPC server that the given IPC client is connected to.
 * @param   cl The given IPC client.
 * @return  The IPC server the IPC client is connected to.
 * @ingroup Ecore_IPC_Client_Group
 */
Ecore_Ipc_Server *
ecore_ipc_client_server_get(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_server_get");
	return NULL;
     }
   return (ecore_con_server_data_get(ecore_con_client_server_get(cl->client)));
}

/**
 * Closes the connection and frees memory allocated to the given IPC
 * client.
 * @param   cl The given client.
 * @return  Data associated with the client.
 * @ingroup Ecore_IPC_Client_Group
 */
void *
ecore_ipc_client_del(Ecore_Ipc_Client *cl)
{
   void *data;
   Ecore_Ipc_Server *svr;
   
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_del");
	return NULL;
     }
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);
   data = cl->data;
   svr = ecore_con_server_data_get(ecore_con_client_server_get(cl->client));
   ecore_con_client_del(cl->client);
   svr->clients = _ecore_list_remove(svr->clients, cl);
   if (cl->buf) free(cl->buf);
   free(cl);
   return data;
}

/**
 * Sets the IPC data associated with the given IPC client to @p data.
 * @param   cl   The given IPC client.
 * @param   data The data to associate with the IPC client.
 * @ingroup Ecore_IPC_Client_Group
 */
void
ecore_ipc_client_data_set(Ecore_Ipc_Client *cl, const void *data)
{  
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_data_set");
	return;
     }
   cl->data = (void *)data;
}  

/**
 * Retrieves the data that has been associated with the given IPC client.
 * @param   cl The given client.
 * @return  The data associated with the IPC client.
 * @ingroup Ecore_IPC_Client_Group
 */
void *
ecore_ipc_client_data_get(Ecore_Ipc_Client *cl)
{  
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_data_get");
	return NULL;
     }
   return cl->data;
}

/**
 *  * Returns if SSL support is available
 *  * @return  1 if SSL is available, 0 if it is not.
 *  * @ingroup Ecore_Con_Client_Group
 *  */
int
ecore_ipc_ssl_available_get(void)
{
   return ecore_con_ssl_available_get();
}


static int
_ecore_ipc_event_client_add(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Add *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(ecore_con_client_server_get(e->client)))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Client *cl;
	Ecore_Ipc_Server *svr;
	
	cl = calloc(1, sizeof(Ecore_Ipc_Client));
	if (!cl) return 0;
	svr = ecore_con_server_data_get(ecore_con_client_server_get(e->client));
	ECORE_MAGIC_SET(cl, ECORE_MAGIC_IPC_CLIENT);
	cl->client = e->client;
	ecore_con_client_data_set(cl->client, (void *)cl);
	svr->clients = _ecore_list_append(svr->clients, cl);
	  {
	     Ecore_Ipc_Event_Client_Add *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Add));
	     if (e2)
	       {
		  e2->client = cl;
		  ecore_event_add(ECORE_IPC_EVENT_CLIENT_ADD, e2, NULL, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_client_del(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Del *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(ecore_con_client_server_get(e->client)))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Client *cl;
	
	cl = ecore_con_client_data_get(e->client);
	  {
	     Ecore_Ipc_Event_Client_Del *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Del));
	     if (e2)
	       {
		  e2->client = cl;
		  ecore_event_add(ECORE_IPC_EVENT_CLIENT_DEL, e2, NULL, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_server_add(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Add *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(e->server))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Server *svr;
	
	svr = ecore_con_server_data_get(e->server);
	  {
	     Ecore_Ipc_Event_Server_Add *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Add));
	     if (e2)
	       {
		  e2->server = svr;
		  ecore_event_add(ECORE_IPC_EVENT_SERVER_ADD, e2, NULL, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_server_del(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Del *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(e->server))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Server *svr;
	
	svr = ecore_con_server_data_get(e->server);
	  {
	     Ecore_Ipc_Event_Server_Del *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Del));
	     if (e2)
	       {
		  e2->server = svr;
		  ecore_event_add(ECORE_IPC_EVENT_SERVER_DEL, e2, NULL, NULL);
	       }
	  }
     }
   return 0;
}

#define CLSZ(_n) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) s += 4; \
   else if (md >= DLT_ADD16) s += 2; \
   else if (md >= DLT_ADD8) s += 1;

#define CLDEC(_n, _member) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) \
     { \
	unsigned int v; \
	unsigned char *dv; \
	dv = (unsigned char *)&v; \
	dv[0] = *(cl->buf + offset + s + 0); \
	dv[1] = *(cl->buf + offset + s + 1); \
	dv[2] = *(cl->buf + offset + s + 2); \
	dv[3] = *(cl->buf + offset + s + 3); \
	d = (int)ntohl(d); \
	s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
	unsigned short v; \
	unsigned char *dv; \
	dv = (unsigned char *)&v; \
	dv[0] = *(cl->buf + offset + s + 0); \
	dv[1] = *(cl->buf + offset + s + 1); \
	d = (int)ntohs(v); \
	s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
	unsigned char v; \
	unsigned char *dv; \
	dv = (unsigned char *)&v; \
	dv[0] = *(cl->buf + offset + s + 0); \
	d = (int)v; \
	s += 1; \
     } \
   msg._member = _ecore_ipc_ddlt_int(d, cl->prev.i._member, md);

static int
_ecore_ipc_event_client_data(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(ecore_con_client_server_get(e->client)))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Client *cl;
	Ecore_Ipc_Msg_Head msg;
	int offset = 0;
	unsigned char *buf;
	
	cl = ecore_con_client_data_get(e->client);
	
	if (!cl->buf)
	  {
	     cl->buf_size = e->size;
	     cl->buf = e->data;
	     e->data = NULL; /* take it out of the old event */
	  }
	else
	  {
	     unsigned char *buf;
	     
	     buf = realloc(cl->buf, cl->buf_size + e->size);
	     if (!buf)
	       {
		  free(cl->buf);
		  cl->buf = 0;
		  cl->buf_size  = 0;
		  return 0;
	       }
	     cl->buf = buf;
	     memcpy(cl->buf + cl->buf_size, e->data, e->size);
	     cl->buf_size += e->size;
	  }
	/* examine header */
	redo:
	if ((cl->buf_size - offset) >= (int)sizeof(int))
	  {
	     int s, md, d, head;
	     unsigned char *dd;
	     
	     dd = (unsigned char *)&head;
	     dd[0] = *(cl->buf + offset + 0);
	     dd[1] = *(cl->buf + offset + 1);
	     dd[2] = *(cl->buf + offset + 2);
	     dd[3] = *(cl->buf + offset + 3);
	     head = ntohl(head);
	     dd = (unsigned char *)&d;
	     s = 4;
	     CLSZ(0);
	     CLSZ(1);
	     CLSZ(2);
	     CLSZ(3);
	     CLSZ(4);
	     CLSZ(5);
	     if ((cl->buf_size - offset) < s)
	       {
		  if (offset > 0) goto scroll;
		  return 0;
	       }
	     
	     s = 4;
	     CLDEC(0, major);
	     CLDEC(1, minor);
	     CLDEC(2, ref);
	     CLDEC(3, ref_to);
	     CLDEC(4, response);
	     CLDEC(5, size);
	     if (msg.size < 0) msg.size = 0;
	     /* there is enough data in the buffer for a full message */
	     if ((cl->buf_size - offset) >= (s + msg.size))
	       {
		  Ecore_Ipc_Event_Client_Data *e2;
		  
		  buf = NULL;
		  if (msg.size > 0)
		    {
		       buf = malloc(msg.size);
		       if (!buf) return 0;
		       memcpy(buf, cl->buf + offset + s, msg.size);
		    }
		  e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Data));
		  if (e2)
		    {
		       e2->client   = cl;
		       e2->major    = msg.major;
		       e2->minor    = msg.minor;
		       e2->ref      = msg.ref;
		       e2->ref_to   = msg.ref_to;
		       e2->response = msg.response;
		       e2->size     = msg.size;
		       e2->data     = buf;
		       ecore_event_add(ECORE_IPC_EVENT_CLIENT_DATA, e2,
				       _ecore_ipc_event_client_data_free, NULL);
		    }
		  cl->prev.i = msg;
		  offset += (s + msg.size);
		  if (cl->buf_size == offset)
		    {
		       free(cl->buf);
		       cl->buf = NULL;
		       cl->buf_size = 0;
		       return 0;
		    }
		  goto redo;
	       }
	     else goto scroll;
	  }
	else
	  {
	     scroll:
	     buf = malloc(cl->buf_size - offset);
	     if (!buf)
	       {
		  free(cl->buf);
		  cl->buf = NULL;
		  cl->buf_size = 0;
		  return 0;
	       }
	     memcpy(buf, cl->buf + offset, cl->buf_size - offset);
	     free(cl->buf);
	     cl->buf = buf;
	     cl->buf_size -= offset;
	  }
     }
   return 0;
}

#define SVSZ(_n) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) s += 4; \
   else if (md >= DLT_ADD16) s += 2; \
   else if (md >= DLT_ADD8) s += 1;

#define SVDEC(_n, _member) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) \
     { \
	unsigned int v; \
	unsigned char *dv; \
	dv = (unsigned char *)&v; \
	dv[0] = *(svr->buf + offset + s + 0); \
	dv[1] = *(svr->buf + offset + s + 1); \
	dv[2] = *(svr->buf + offset + s + 2); \
	dv[3] = *(svr->buf + offset + s + 3); \
	d = (int)ntohl(d); \
	s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
	unsigned short v; \
	unsigned char *dv; \
	dv = (unsigned char *)&v; \
	dv[0] = *(svr->buf + offset + s + 0); \
	dv[1] = *(svr->buf + offset + s + 1); \
	d = (int)ntohs(v); \
	s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
	unsigned char v; \
	unsigned char *dv; \
	dv = (unsigned char *)&v; \
	dv[0] = *(svr->buf + offset + s + 0); \
	d = (int)v; \
	s += 1; \
     } \
   msg._member = _ecore_ipc_ddlt_int(d, svr->prev.i._member, md);

static int
_ecore_ipc_event_server_data(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(e->server))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Server *svr;
	Ecore_Ipc_Msg_Head msg;
	int offset = 0;
	unsigned char *buf;
	
	svr = ecore_con_server_data_get(e->server);
	
	if (!svr->buf)
	  {
	     svr->buf_size = e->size;
	     svr->buf = e->data;
	     e->data = NULL; /* take it out of the old event */
	  }
	else
	  {
	     unsigned char *buf;
	     
	     buf = realloc(svr->buf, svr->buf_size + e->size);
	     if (!buf)
	       {
		  free(svr->buf);
		  svr->buf = 0;
		  svr->buf_size  = 0;
		  return 0;
	       }
	     svr->buf = buf;
	     memcpy(svr->buf + svr->buf_size, e->data, e->size);
	     svr->buf_size += e->size;
	  }
	/* examine header */
	redo:
	if ((svr->buf_size - offset) >= (int)sizeof(int))
	  {
	     int s, md, d, head;
	     unsigned char *dd;
	     
	     dd = (unsigned char *)&head;
	     dd[0] = *(svr->buf + offset + 0);
	     dd[1] = *(svr->buf + offset + 1);
	     dd[2] = *(svr->buf + offset + 2);
	     dd[3] = *(svr->buf + offset + 3);
	     head = ntohl(head);
	     dd = (unsigned char *)&d;
	     s = 4;
	     SVSZ(0);
	     SVSZ(1);
	     SVSZ(2);
	     SVSZ(3);
	     SVSZ(4);
	     SVSZ(5);
	     if ((svr->buf_size - offset) < s)
	       {
		  if (offset > 0) goto scroll;
		  return 0;
	       }
	     
	     s = 4;
	     SVDEC(0, major);
	     SVDEC(1, minor);
	     SVDEC(2, ref);
	     SVDEC(3, ref_to);
	     SVDEC(4, response);
	     SVDEC(5, size);
	     if (msg.size < 0) msg.size = 0;
	     /* there is enough data in the buffer for a full message */
	     if ((svr->buf_size - offset) >= (s + msg.size))
	       {
		  Ecore_Ipc_Event_Server_Data *e2;
		  
		  buf = NULL;
		  if (msg.size > 0)
		    {
		       buf = malloc(msg.size);
		       if (!buf) return 0;
		       memcpy(buf, svr->buf + offset + s, msg.size);
		    }
		  e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Data));
		  if (e2)
		    {
		       e2->server   = svr;
		       e2->major    = msg.major;
		       e2->minor    = msg.minor;
		       e2->ref      = msg.ref;
		       e2->ref_to   = msg.ref_to;
		       e2->response = msg.response;
		       e2->size     = msg.size;
		       e2->data     = buf;
		       ecore_event_add(ECORE_IPC_EVENT_SERVER_DATA, e2,
				       _ecore_ipc_event_server_data_free, NULL);
		    }
		  svr->prev.i = msg;
		  offset += (s + msg.size);
		  if (svr->buf_size == offset)
		    {
		       free(svr->buf);
		       svr->buf = NULL;
		       svr->buf_size = 0;
		       return 0;
		    }
		  goto redo;
	       }
	     else goto scroll;
	  }
	else
	  {
	     scroll:
	     buf = malloc(svr->buf_size - offset);
	     if (!buf)
	       {
		  free(svr->buf);
		  svr->buf = NULL;
		  svr->buf_size = 0;
		  return 0;
	       }
	     memcpy(buf, svr->buf + offset, svr->buf_size - offset);
	     free(svr->buf);
	     svr->buf = buf;
	     svr->buf_size -= offset;
	  }
     }
   return 0;
}

static void
_ecore_ipc_event_client_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Ipc_Event_Client_Data *e;
   
   e = ev;
   if (e->data) free(e->data);
   free(e);
}

static void
_ecore_ipc_event_server_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Ipc_Event_Server_Data *e;
   
   e = ev;
   if (e->data) free(e->data);
   free(e);
}
