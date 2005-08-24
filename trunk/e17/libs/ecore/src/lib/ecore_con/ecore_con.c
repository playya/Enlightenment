/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Ecore.h"
#include "config.h"
#include "ecore_private.h"
#include "ecore_con_private.h"
#include "Ecore_Con.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#elif WIN32
#include <winsock.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>

#if USE_OPENSSL
#include <time.h>
#endif

static void _ecore_con_cb_dns_lookup(struct hostent *he, void *data);
static void _ecore_con_server_free(Ecore_Con_Server *svr);
static void _ecore_con_client_free(Ecore_Con_Client *cl);
static int _ecore_con_svr_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_cl_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler);
static void _ecore_con_server_flush(Ecore_Con_Server *svr);
static void _ecore_con_client_flush(Ecore_Con_Client *cl);
static void _ecore_con_event_client_data_free(void *data, void *ev);
static void _ecore_con_event_server_data_free(void *data, void *ev);

int ECORE_CON_EVENT_CLIENT_ADD = 0;
int ECORE_CON_EVENT_CLIENT_DEL = 0;
int ECORE_CON_EVENT_SERVER_ADD = 0;
int ECORE_CON_EVENT_SERVER_DEL = 0;
int ECORE_CON_EVENT_CLIENT_DATA = 0;
int ECORE_CON_EVENT_SERVER_DATA = 0;

static Ecore_List *servers = NULL;
static int init_count = 0;

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

/**
 * @defgroup Ecore_Con_Lib_Group Ecore Connection Library Functions
 *
 * Utility functions that set up and shut down the Ecore Connection
 * library.
 */

/**
 * Initialises the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @ingroup Ecore_Con_Lib_Group
 */
int
ecore_con_init(void)
{
   init_count++;
   if (!ECORE_CON_EVENT_CLIENT_ADD)
     {
	ECORE_CON_EVENT_CLIENT_ADD = ecore_event_type_new();
	ECORE_CON_EVENT_CLIENT_DEL = ecore_event_type_new();
	ECORE_CON_EVENT_SERVER_ADD = ecore_event_type_new();
	ECORE_CON_EVENT_SERVER_DEL = ecore_event_type_new();
	ECORE_CON_EVENT_CLIENT_DATA = ecore_event_type_new();
	ECORE_CON_EVENT_SERVER_DATA = ecore_event_type_new();

#if USE_OPENSSL
	SSL_library_init();
	SSL_load_error_strings();
#endif

	/* TODO Remember return value, if it fails, use gethostbyname() */
	ecore_con_dns_init();
     }
   if (!servers)
      servers = ecore_list_new();
   return init_count;
}

/**
 * Shuts down the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @ingroup Ecore_Con_Lib_Group
 */
int
ecore_con_shutdown(void)
{
   if (init_count > 0)
     {
	init_count--;
	if (init_count > 0) return init_count;
	while (!ecore_list_is_empty(servers))
	     _ecore_con_server_free(ecore_list_remove_first(servers));
	ecore_list_destroy(servers);
	servers = NULL;

	ecore_con_dns_shutdown();
     }
   return 0;
}

/**
 * @defgroup Ecore_Con_Server_Group Ecore Connection Server Functions
 *
 * Functions that operate on Ecore server objects.
 */

/**
 * Creates a server to listen for connections.
 *
 * The socket on which the server listens depends on the connection
 * type:
 * @li If @a compl_type is @c ECORE_CON_LOCAL_USER, the server will listen on
 *     the Unix socket "~/.ecore/[name]/[port]".
 * @li If @a compl_type is @c ECORE_CON_LOCAL_SYSTEM, the server will listen
 *     on Unix socket "/tmp/.ecore_service|[name]|[port]".
 * @li If @a compl_type is @c ECORE_CON_REMOTE_SYSTEM, the server will listen
 *     on TCP port @c port.
 *
 * @param  compl_type The connection type.
 * @param  name       Name to associate with the socket.  It is used when
 *                    generating the socket name of a Unix socket.  Though
 *                    it is not used for the TCP socket, it still needs to
 *                    be a valid character array.  @c NULL will not be
 *                    accepted.
 * @param  port       Number to identify socket.  When a Unix socket is used,
 *                    it becomes part of the socket name.  When a TCP socket
 *                    is used, it is used as the TCP port.
 * @param  data       Data to associate with the created Ecore_Con_Server
 *                    object.
 * @return A new Ecore_Con_Server.
 * @ingroup Ecore_Con_Server_Group
 */
Ecore_Con_Server *
ecore_con_server_add(Ecore_Con_Type compl_type,
		     const char *name,
		     int port,
		     const void *data)
{
   Ecore_Con_Server   *svr;
   Ecore_Con_Type      type;
   struct sockaddr_in  socket_addr;
   struct sockaddr_un  socket_unix;
   struct linger       lin;
   char                buf[4096];
   
   if (port < 0) return NULL;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr) return NULL;

   type = compl_type;
#if USE_OPENSSL
   /* unset the SSL flag for the following checks */
   type &= ~ECORE_CON_USE_SSL;
#endif
   
   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM))
     {
	const char *homedir;
	struct stat st;
	mode_t pmode, mask;
	
	if (!name) goto error;
	mask =
	  S_IRGRP | S_IWGRP | S_IXGRP |
	  S_IROTH | S_IWOTH | S_IXOTH;
	if (type == ECORE_CON_LOCAL_USER)
	  {
	     homedir = getenv("HOME");
	     if (!homedir) homedir = getenv("TMP");
	     if (!homedir) homedir = "/tmp";
	     mask = S_IRUSR | S_IWUSR | S_IXUSR;
	     snprintf(buf, sizeof(buf), "%s/.ecore", homedir);
	     if (stat(buf, &st) < 0) mkdir(buf, mask);
	     snprintf(buf, sizeof(buf), "%s/.ecore/%s", homedir, name);
	     if (stat(buf, &st) < 0) mkdir(buf, mask);
	     snprintf(buf, sizeof(buf), "%s/.ecore/%s/%i", homedir, name, port);
	     mask =
	       S_IRGRP | S_IWGRP | S_IXGRP |
	       S_IROTH | S_IWOTH | S_IXOTH;
	  }
	else if (type == ECORE_CON_LOCAL_SYSTEM)
	  {
	     mask = 0;
        if (name[0] == '/')
          snprintf(buf, sizeof(buf), "%s|%i", name, port);
        else
          snprintf(buf, sizeof(buf), "/tmp/.ecore_service|%s|%i", name, port);
	  }
	pmode = umask(mask);
	start:
	svr->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (svr->fd < 0)
	  {
	     umask(pmode);
	     goto error;
	  }
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	lin.l_onoff = 1;
	lin.l_linger = 100;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	socket_unix.sun_family = AF_UNIX;
	strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
	if (bind(svr->fd, (struct sockaddr *)&socket_unix, LENGTH_OF_SOCKADDR_UN(&socket_unix)) < 0)
	  {
	     if (connect(svr->fd, (struct sockaddr *)&socket_unix, 
			 LENGTH_OF_SOCKADDR_UN(&socket_unix)) < 0)
	       {
		  if ((type == ECORE_CON_LOCAL_USER) ||
		      (type == ECORE_CON_LOCAL_SYSTEM))
		    {
		       if (unlink(buf) < 0)
			 {
			    umask(pmode);
			    goto error;		       
			 }
		       else
			 goto start;
		    }
		  else
		    {
		       umask(pmode);
		       goto error;		       
		    }
	       }
	     else
	       {
		  umask(pmode);	     
		  goto error;
	       }
	  }
	if (listen(svr->fd, 4096) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	svr->path = strdup(buf);
	if (!svr->path)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ,
						    _ecore_con_svr_handler, svr,
						    NULL, NULL);
	umask(pmode);
	if (!svr->fd_handler) goto error;
     }
   else if (type == ECORE_CON_REMOTE_SYSTEM)
     {
	svr->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (svr->fd < 0) goto error;
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
	lin.l_onoff = 1;
	lin.l_linger = 100;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0) goto error;
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(svr->fd, (struct sockaddr *)&socket_addr, sizeof(struct sockaddr_in)) < 0) goto error;
	if (listen(svr->fd, 4096) < 0) goto error;
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ,
						    _ecore_con_svr_handler, svr,
						    NULL, NULL);
	if (!svr->fd_handler) goto error;
     }

#if USE_OPENSSL
   if (compl_type & ECORE_CON_USE_SSL)
     {
	/* SSLv3 gives *weird* results on my box, don't use it yet */
	if (!(svr->ssl_ctx = SSL_CTX_new(SSLv2_client_method())))
	  goto error;
	
	if (!(svr->ssl = SSL_new(svr->ssl_ctx)))
	  goto error;
	
	SSL_set_fd(svr->ssl, svr->fd);
     }
#endif
   
   svr->name = strdup(name);
   if (!svr->name) goto error;
   svr->type = type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = 1;
   svr->reject_excess_clients = 0;
   svr->client_limit = -1;
   svr->clients = ecore_list_new();
   ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);   
   return svr;
   
   error:
   if (svr->name) free(svr->name);
   if (svr->path) free(svr->path);
   if (svr->fd >= 0) close(svr->fd);
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
   if (svr->write_buf) free(svr->write_buf);
#if USE_OPENSSL
   if (svr->ssl) SSL_free(svr->ssl);
   if (svr->ssl_ctx) SSL_CTX_free(svr->ssl_ctx);
#endif
   free(svr);
   return NULL;
}

/**
 * Creates a server object to represent the server listening at the
 * given port.
 *
 * The socket to which the server connects depends on the connection type:
 * @li If @a compl_type is @c ECORE_CON_LOCAL_USER, the function will
 *     connect to the server listening on the Unix socket
 *     "~/.ecore/[name]/[port]".
 * @li If @a compl_type is @c ECORE_CON_LOCAL_SYSTEM, the function will
 *     connect to the server listening on the Unix socket
 *     "/tmp/.ecore_service|[name]|[port]".
 * @li If @a compl_type is @c ECORE_CON_REMOTE_SYSTEM, the function will
 *     connect to the server listening on the TCP port "[name]:[port]".
 *
 * @param  compl_type The connection type.
 * @param  name       Name used when determining what socket to connect to.
 *                    It is used to generate the socket name when the socket
 *                    is a Unix socket.  It is used as the hostname when
 *                    connecting with a TCP socket.
 * @param  port       Number to identify the socket to connect to.  Used when
 *                    generating the socket name for a Unix socket, or as the
 *                    TCP port when connecting to a TCP socket.
 * @param  data       Data to associate with the created Ecore_Con_Server
 *                    object.
 * @return A new Ecore_Con_Server.
 * @ingroup Ecore_Con_Server_Group
 */
Ecore_Con_Server *
ecore_con_server_connect(Ecore_Con_Type compl_type,
			 const char *name,
			 int port,
			 const void *data)
{
   Ecore_Con_Server   *svr;
   Ecore_Con_Type      type;
   struct sockaddr_un  socket_unix;
   int                 curstate = 0;
   char                buf[4096];

   if (!name) return NULL;
   if (port < 0) return NULL;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr) return NULL;
   
   type = compl_type;
#if USE_OPENSSL
   /* unset the SSL flag for the following checks */
   type &= ~ECORE_CON_USE_SSL;
#endif

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM))
     {
	const char *homedir;
	
	if (type == ECORE_CON_LOCAL_USER)
	  {
	     homedir = getenv("HOME");
	     if (!homedir) homedir = getenv("TMP");
	     if (!homedir) homedir = "/tmp";
	     snprintf(buf, sizeof(buf), "%s/.ecore/%s/%i", homedir, name, port);
	  }
	else if (type == ECORE_CON_LOCAL_SYSTEM)
	  {
	     if (name[0] == '/')
	       snprintf(buf, sizeof(buf), "%s|%i", name, port);
	     else
	       snprintf(buf, sizeof(buf), "/tmp/.ecore_service|%s|%i", name, port);
	  }
	svr->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (svr->fd < 0) goto error;
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate)) < 0) goto error;
	socket_unix.sun_family = AF_UNIX;
	strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
	if (connect(svr->fd, (struct sockaddr *)&socket_unix, LENGTH_OF_SOCKADDR_UN(&socket_unix)) < 0) goto error;
	svr->path = strdup(buf);
	if (!svr->path) goto error;
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ,
						    _ecore_con_cl_handler, svr,
						    NULL, NULL);
	if (!svr->fd_handler) goto error;
	  {
	     /* we got our server! */
	     Ecore_Con_Event_Server_Add *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Server_Add));
	     if (e)
	       {
		  e->server = svr;
		  ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, e, NULL, NULL);
	       }
	  }
     }
   else if (type == ECORE_CON_REMOTE_SYSTEM)
     {
	ecore_con_dns_lookup(name, _ecore_con_cb_dns_lookup, svr);
     }

   svr->name = strdup(name);
   if (!svr->name) goto error;
   svr->type = compl_type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = 0;
   svr->reject_excess_clients = 0;
   svr->client_limit = -1;
   svr->clients = ecore_list_new();
   ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);   
   return svr;
   
   error:
   if (svr->name) free(svr->name);
   if (svr->path) free(svr->path);
   if (svr->fd >= 0) close(svr->fd);
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
#if USE_OPENSSL
   if (svr->ssl) SSL_free(svr->ssl);
   if (svr->ssl_ctx) SSL_CTX_free(svr->ssl_ctx);
#endif
   free(svr);
   return NULL;
}

/**
 * Closes the connection and frees the given server.
 * @param   svr The given server.
 * @return  Data associated with the server when it was created.
 * @ingroup Ecore_Con_Server_Group
 */
void *
ecore_con_server_del(Ecore_Con_Server *svr)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_del");
	return NULL;
     }   
   data = svr->data;
   _ecore_con_server_free(svr);
   if (ecore_list_goto(servers, svr)) ecore_list_remove(servers);
   return data;
}

/**
 * Retrieves the data associated with the given server.
 * @param   svr The given server.
 * @return  The associated data.
 * @ingroup Ecore_Con_Server_Group
 */
void *
ecore_con_server_data_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_data_get");
	return NULL;
     }   
   return svr->data;
}

/**
 * Retrieves whether the given server is currently connected.
 * @todo Check that this function does what the documenter believes it does.
 * @param   svr The given server.
 * @return  @c 1 if the server is connected.  @c 0 otherwise.
 * @ingroup Ecore_Con_Server_Group
 */
int
ecore_con_server_connected_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_connected_get");
	return 0;
     }   
   if (svr->connecting) return 0;
   return 1;
}

/**
 * Sends the given data to the given server.
 * @param   svr  The given server.
 * @param   data The given data.
 * @param   size Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is an
 *          error.
 * @ingroup Ecore_Con_Server_Group
 */
int
ecore_con_server_send(Ecore_Con_Server *svr, void *data, int size)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_send");
	return 0;
     }   
   if (svr->dead) return 0;
   if (!data) return 0;
   if (size < 1) return 0;
   ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
   if (svr->write_buf)
     {
	unsigned char *newbuf;
	
	newbuf = realloc(svr->write_buf, svr->write_buf_size + size);
	if (newbuf) svr->write_buf = newbuf;
	else return 0;
	memcpy(svr->write_buf + svr->write_buf_size, data, size);
	svr->write_buf_size += size;
     }
   else
     {
	svr->write_buf = malloc(size);
	if (!svr->write_buf) return 0;
	svr->write_buf_size = size;
	memcpy(svr->write_buf, data, size);
     }
   return size;
}

/**
 * Sets a limit on the number of clients that can be handled concurrently
 * by the given server, and a policy on what to do if excess clients try to
 * connect.
 * Beware that if you set this once ecore is already running, you may
 * already have pending CLIENT_ADD events in your event queue.  Those
 * clients have already connected and will not be affected by this call.
 * Only clients subsequently trying to connect will be affected.
 * @param   svr           The given server.
 * @param   client_limit  The maximum number of clients to handle
 *                        concurrently.  -1 means unlimited (default).  0 
 *                        effectively disables the server.
 * @param   reject_excess_clients  Set to 1 to automatically disconnect
 *                        excess clients as soon as they connect if you are
 *                        already handling client_limit clients.  Set to 0
 *                        (default) to just hold off on the "accept()"
 *                        system call until the number of active clients
 *                        drops. This causes the kernel to queue up to 4096
 *                        connections (or your kernel's limit, whichever is
 *                        lower).
 * @ingroup Ecore_Con_Server_Group
 */
void
ecore_con_server_client_limit_set(Ecore_Con_Server *svr, int client_limit, char reject_excess_clients)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_client_limit_set");
	return;
     }   
   svr->client_limit = client_limit;
   svr->reject_excess_clients = reject_excess_clients;
}

/**
 * @defgroup Ecore_Con_Client_Group Ecore Connection Client Functions
 *
 * Functions that operate on Ecore connection client objects.
 */

/**
 * Sends the given data to the given client.
 * @param   cl   The given client.
 * @param   data The given data.
 * @param   size Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is an
 *          error.
 * @ingroup Ecore_Con_Client_Group
 */
int
ecore_con_client_send(Ecore_Con_Client *cl, void *data, int size)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_send");
	return 0;
     }   
   if (cl->dead) return 0;
   if (!data) return 0;
   if (size < 1) return 0;
   ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
   if (cl->buf)
     {
	unsigned char *newbuf;
	
	newbuf = realloc(cl->buf, cl->buf_size + size);
	if (newbuf) cl->buf = newbuf;
	else return 0;
	memcpy(cl->buf + cl->buf_size, data, size);
	cl->buf_size += size;
     }
   else
     {
	cl->buf = malloc(size);
	if (!cl->buf) return 0;
	cl->buf_size = size;
	memcpy(cl->buf, data, size);
     }
   return size;
}
  
/**
 * Retrieves the server representing the socket the client has
 * connected to.
 * @param   cl The given client.  
 * @return  The server that the client connected to.
 * @ingroup Ecore_Con_Client_Group
 */
Ecore_Con_Server *
ecore_con_client_server_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_server_get");
	return NULL;
     }   
   return cl->server;
}

/**
 * Closes the connection and frees memory allocated to the given client.
 * @param   cl The given client.
 * @return  Data associated with the client.
 * @ingroup Ecore_Con_Client_Group
 */
void *
ecore_con_client_del(Ecore_Con_Client *cl)
{
   void *data;
   
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_del");
	return NULL;
     }   
   data = cl->data;
   if (ecore_list_goto(cl->server->clients, cl))
     ecore_list_remove(cl->server->clients);
   _ecore_con_client_free(cl);
   return data;
}

/**
 * Sets the data associated with the given client to @p data.
 * @param   cl   The given client.
 * @param   data What to set the data to.
 * @ingroup Ecore_Con_Client_Group
 */
void
ecore_con_client_data_set(Ecore_Con_Client *cl, const void *data)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_data_set");
	return;
     }
   cl->data = (void *)data;
}

/**
 * Retrieves the data associated with the given client.
 * @param   cl The given client.
 * @return  The data associated with @p cl.
 * @ingroup Ecore_Con_Client_Group
 */
void *
ecore_con_client_data_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_data_get");
	return NULL;
     }
   return cl->data;
}

/**
 * Returns if SSL support is available
 * @return  1 if SSL is available, 0 if it is not.
 * @ingroup Ecore_Con_Client_Group
 */
int
ecore_con_ssl_available_get(void)
{
#if USE_OPENSSL
   return 1;
#else
   return 0;
#endif   
}

static void
_ecore_con_server_free(Ecore_Con_Server *svr)
{
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);   
   while ((svr->write_buf) && (!svr->dead)) _ecore_con_server_flush(svr);
   if (svr->write_buf) free(svr->write_buf);
   while (!ecore_list_is_empty(svr->clients))
      _ecore_con_client_free(ecore_list_remove_first(svr->clients));
   ecore_list_destroy(svr->clients);
   if ((svr->created) && (svr->path)) unlink(svr->path);
   if (svr->fd >= 0) close(svr->fd);
#if USE_OPENSSL
   if (svr->ssl)
     {
	SSL_shutdown(svr->ssl);
	SSL_free(svr->ssl);
     }
   if (svr->ssl_ctx) SSL_CTX_free(svr->ssl_ctx);
#endif
   if (svr->name) free(svr->name);
   if (svr->path) free(svr->path);
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
   free(svr);
}

static void
_ecore_con_client_free(Ecore_Con_Client *cl)
{
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);   
   while ((cl->buf) && (!cl->dead)) _ecore_con_client_flush(cl);
   if (cl->buf) free(cl->buf);
   if (cl->fd >= 0) close(cl->fd);
   if (cl->fd_handler) ecore_main_fd_handler_del(cl->fd_handler);
   free(cl);
}

static int
_ecore_con_svr_handler(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   Ecore_Con_Server   *svr;
   int                 new_fd;
   struct sockaddr_in  incoming;
   size_t              size_in;
   
   svr = data;
   if (svr->dead) return 1;
   if ((svr->client_limit >= 0) && (!svr->reject_excess_clients))
     {
	if (ecore_list_nodes(svr->clients) >= svr->client_limit) return 1;
     }
   /* a new client */
   size_in = sizeof(struct sockaddr_in);
   new_fd = accept(svr->fd, (struct sockaddr *)&incoming, &size_in);
   if (new_fd >= 0)
     {
	Ecore_Con_Client *cl;

	if ((svr->client_limit >= 0) && (svr->reject_excess_clients))
	  {
	     close(new_fd);
	     return 1;
	  }

	cl = calloc(1, sizeof(Ecore_Con_Client));
	if (!cl)
	  {
	     close(new_fd);
	     return 1;
	  }
	fcntl(new_fd, F_SETFL, O_NONBLOCK);
	fcntl(new_fd, F_SETFD, FD_CLOEXEC);
	cl->fd = new_fd;
	cl->server = svr;
	cl->fd_handler = ecore_main_fd_handler_add(cl->fd,
						   ECORE_FD_READ,
						   _ecore_con_svr_cl_handler, 
						   cl, NULL, NULL);
	ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);
	ecore_list_append(svr->clients, cl);
	  {
	     Ecore_Con_Event_Client_Add *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Client_Add));
	     if (e)
	       {
		  e->client = cl;
		  ecore_event_add(ECORE_CON_EVENT_CLIENT_ADD, e, NULL, NULL);
	       }
	  }
     }
   return 1;
}

#if USE_OPENSSL
/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static int
svr_try_connect_ssl(Ecore_Con_Server *svr)
{
   int res, ssl_err, flag = 0;
   
   res = SSL_connect(svr->ssl);
   if (res == 1) return 1;
   ssl_err = SSL_get_error(svr->ssl, res);
   
   if (ssl_err == SSL_ERROR_NONE) return 1;
   if (ssl_err == SSL_ERROR_WANT_READ)       flag = ECORE_FD_READ;
   else if (ssl_err == SSL_ERROR_WANT_WRITE) flag = ECORE_FD_WRITE;
   else return -1;
   if (flag) ecore_main_fd_handler_active_set(svr->fd_handler, flag);
   return 0;
}
#endif

static void
kill_server(Ecore_Con_Server *svr)
{
   Ecore_Con_Event_Server_Del *e;
   
   e = calloc(1, sizeof(Ecore_Con_Event_Server_Del));
   if (e)
     {
	e->server = svr;
	ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e, NULL, NULL);
     }
   
   svr->dead = 1;
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
   svr->fd_handler = NULL;
}

static void
_ecore_con_cb_dns_lookup(struct hostent *he, void *data)
{
   Ecore_Con_Server   *svr;
   struct sockaddr_in  socket_addr;
   int                 curstate = 0;

   svr = data;

   if (!he) goto error;
   svr->fd = socket(AF_INET, SOCK_STREAM, 0);
   if (svr->fd < 0) goto error;
   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate)) < 0) goto error;
   socket_addr.sin_family = AF_INET;
   socket_addr.sin_port = htons(svr->port);
   memcpy((struct in_addr *)&socket_addr.sin_addr, 
	  he->h_addr, sizeof(struct in_addr));
   if (connect(svr->fd, (struct sockaddr *)&socket_addr, sizeof(struct sockaddr_in)) < 0) 
     {
	if (errno != EINPROGRESS)
	  goto error;
	svr->connecting = 1;
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ | ECORE_FD_WRITE,
						    _ecore_con_cl_handler, svr,
						    NULL, NULL);
     }
   else
     svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						 ECORE_FD_READ,
						 _ecore_con_cl_handler, svr,
						 NULL, NULL);

   if (!svr->fd_handler) goto error;

#if USE_OPENSSL
   if (svr->type & ECORE_CON_USE_SSL)
     {
	/* SSLv3 gives *weird* results on my box, don't use it yet */
	if (!(svr->ssl_ctx = SSL_CTX_new(SSLv2_client_method())))
	  goto error;
	
	if (!(svr->ssl = SSL_new(svr->ssl_ctx)))
	  goto error;
	
	SSL_set_fd(svr->ssl, svr->fd);
     }
#endif

   return;

   error:
   kill_server(svr);
}

static int
svr_try_connect_plain(Ecore_Con_Server *svr)
{
   int so_err = 0;
   unsigned int size = sizeof(int);

   if (getsockopt(svr->fd, SOL_SOCKET, SO_ERROR, &so_err, &size) < 0)
     so_err = -1;
   
   if (so_err != 0)
     {
	/* we lost our server! */
	kill_server(svr);
     }
   else
     {
	/* we got our server! */
	Ecore_Con_Event_Server_Add *e;
	
	svr->connecting = 0;
	e = calloc(1, sizeof(Ecore_Con_Event_Server_Add));
	if (e)
	  {
	     e->server = svr;
	     ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, e, NULL, NULL);
	  }
	if (!svr->write_buf)
	  ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
     }
   return (!svr->dead);
}

/* returns 1 on success, 0 on failure */
static int svr_try_connect(Ecore_Con_Server *svr)
{
#if USE_OPENSSL
   if (!svr->ssl)
   {
#endif
      return svr_try_connect_plain(svr);
#if USE_OPENSSL
   }
   else
      switch (svr_try_connect_ssl(svr)) {
	 case 1:
	    return svr_try_connect_plain(svr);
	 case -1:
	    kill_server(svr);
	    return 0;
	 default:
	    return 0;
      }
#endif
}


static int
_ecore_con_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server   *svr;
#if USE_OPENSSL
   int ssl_err = SSL_ERROR_NONE;
#endif
   
   svr = data;
   if (svr->dead) return 1;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
	unsigned char *inbuf = NULL;
	int            inbuf_num = 0;

	if (svr->connecting && !svr_try_connect(svr))
	   return 1;

	for (;;)
	  {
	     int num, lost_server = 0;
	     char buf[READBUFSIZ];

#if USE_OPENSSL
	     if (!svr->ssl)
	       {
#endif
		  if ((num = read(svr->fd, buf, READBUFSIZ)) < 1)
		    lost_server = (errno == EIO || errno == EBADF ||
				   errno == EPIPE || errno == EINVAL ||
				   errno == ENOSPC || num == 0); /* is num == 0 right? */
#if USE_OPENSSL
	       }
	     else
	       {
		  num = SSL_read(svr->ssl, buf, READBUFSIZ);
		  if (num < 1)
		    {
		       ssl_err = SSL_get_error(svr->ssl, num);
		       lost_server = (ssl_err == SSL_ERROR_ZERO_RETURN);
		    }
		  else
		    ssl_err = SSL_ERROR_NONE;
	       }
#endif
	     if (num < 1)
	       {
		  if (inbuf) 
		    {
		       Ecore_Con_Event_Server_Data *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Server_Data));
		       if (e)
			 {
			    e->server = svr;
			    e->data = inbuf;
			    e->size = inbuf_num;
			    ecore_event_add(ECORE_CON_EVENT_SERVER_DATA, e,
					    _ecore_con_event_server_data_free, NULL);
			 }
		    }
		  if (lost_server)
		    {
		       /* we lost our server! */
		       kill_server(svr);
		       return 1;
		    }
		  break;
	       }
	     else
	       {
		  inbuf = realloc(inbuf, inbuf_num + num);
		  memcpy(inbuf + inbuf_num, buf, num);
		  inbuf_num += num;
	       }
	  }

#if USE_OPENSSL
	if (svr->ssl && ssl_err == SSL_ERROR_WANT_READ)
	  ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
	else if (svr->ssl && ssl_err == SSL_ERROR_WANT_WRITE)
	  ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
#endif
     }
   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     {
	if (svr->connecting && !svr_try_connect (svr))
	   return 1;

	_ecore_con_server_flush(svr);
     }

   return 1;
}

static int
_ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Client   *cl;
   
   cl = data;
   if (cl->dead) return 1;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
	unsigned char *inbuf = NULL;
	int            inbuf_num = 0;
	
	for (;;)
	  {
	     char buf[65536];
	     int num;
	     
	     errno = 0;
	     num = read(cl->fd, buf, 65536);
	     if (num < 1)
	       {
		  if (inbuf) 
		    {
		       Ecore_Con_Event_Client_Data *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Client_Data));
		       if (e)
			 {
			    e->client = cl;
			    e->data = inbuf;
			    e->size = inbuf_num;
			    ecore_event_add(ECORE_CON_EVENT_CLIENT_DATA, e,
					    _ecore_con_event_client_data_free, NULL);
			 }
		    }
		  if ((errno == EIO) ||  (errno == EBADF) || 
		      (errno == EPIPE) || (errno == EINVAL) || 
		      (errno == ENOSPC) || (num == 0)/* is num == 0 right? */)
		    {
		       /* we lost our client! */
		       Ecore_Con_Event_Client_Del *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
		       if (e)
			 {
			    e->client = cl;
			    ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e, NULL, NULL);
			 }
		       cl->dead = 1;
		       ecore_main_fd_handler_del(cl->fd_handler);
		       cl->fd_handler = NULL;
		    }
		  break;
	       }
	     else
	       {
		  inbuf = realloc(inbuf, inbuf_num + num);
		  memcpy(inbuf + inbuf_num, buf, num);
		  inbuf_num += num;
	       }
	  }
     }
   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     _ecore_con_client_flush(cl);
   return 1;
}

static void
_ecore_con_server_flush(Ecore_Con_Server *svr)
{
   int count, num, lost_server = 0;
#if USE_OPENSSL
   int ssl_err = SSL_ERROR_NONE;
#endif

   if (!svr->write_buf) return;

   /* check whether we need to write anything at all.
	* we must not write zero bytes with SSL_write() since it
	* causes undefined behaviour
	*/
   if (svr->write_buf_size == svr->write_buf_offset)
      return;
   
   num = svr->write_buf_size - svr->write_buf_offset;
#if USE_OPENSSL
   if (!svr->ssl)
     {
#endif
	count = write(svr->fd, svr->write_buf + svr->write_buf_offset, num);
	if (count < 1)
	  lost_server = (errno == EIO || errno == EBADF ||
			 errno == EPIPE || errno == EINVAL ||
			 errno == ENOSPC);
#if USE_OPENSSL
     }
   else 
     {
	count = SSL_write(svr->ssl, svr->write_buf + svr->write_buf_offset, num);
	
	if (count < 1)
	  {
	     ssl_err = SSL_get_error(svr->ssl, count);
	     lost_server = (ssl_err == SSL_ERROR_ZERO_RETURN);
	  }
     }
#endif

   if (lost_server)
     {
	/* we lost our server! */
	kill_server(svr);
	return;
     }

   if (count < 1)
     {
#if USE_OPENSSL
	if (svr->ssl && ssl_err == SSL_ERROR_WANT_READ)
	  ecore_main_fd_handler_active_set(svr->fd_handler,
					   ECORE_FD_READ);
	else if (svr->ssl && ssl_err == SSL_ERROR_WANT_WRITE)
	  ecore_main_fd_handler_active_set(svr->fd_handler,
					   ECORE_FD_WRITE);
#endif
	return;
     }
   
   svr->write_buf_offset += count;
   if (svr->write_buf_offset >= svr->write_buf_size)
     {
	svr->write_buf_size = 0;
	svr->write_buf_offset = 0;
	free(svr->write_buf);
	svr->write_buf = NULL;
	ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
     }
}

static void
_ecore_con_client_flush(Ecore_Con_Client *cl)
{
   int count, num;

   if (!cl->buf) return;
   num = cl->buf_size - cl->buf_offset;
   count = write(cl->fd, cl->buf + cl->buf_offset, num);
   if (count < 1)
     {
	if ((errno == EIO) || (errno == EBADF) || (errno == EPIPE) ||
	    (errno == EINVAL) || (errno == ENOSPC))
	  {
	     /* we lost our client! */
	     Ecore_Con_Event_Client_Del *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
	     if (e)
	       {
		  e->client = cl;
		  ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e, NULL, NULL);
	       }
	     cl->dead = 1;
	     ecore_main_fd_handler_del(cl->fd_handler);
	     cl->fd_handler = NULL;
	  }
	return;
     }
   cl->buf_offset += count;
   if (cl->buf_offset >= cl->buf_size)
     {
	cl->buf_size = 0;
	cl->buf_offset = 0;
	free(cl->buf);
	cl->buf = NULL;
	ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
     }
}

static void
_ecore_con_event_client_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (e->data) free(e->data);
   free(e);
}

static void
_ecore_con_event_server_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   if (e->data) free(e->data);
   free(e);
}
