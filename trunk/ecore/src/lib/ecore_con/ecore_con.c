#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

static Eina_Bool _ecore_con_client_timer(Ecore_Con_Client *cl);
static void      _ecore_con_cl_timer_update(Ecore_Con_Client *cl);

static void      _ecore_con_cb_tcp_connect(void *data, Ecore_Con_Info *info);
static void      _ecore_con_cb_udp_connect(void *data, Ecore_Con_Info *info);
static void      _ecore_con_cb_tcp_listen(void *data, Ecore_Con_Info *info);
static void      _ecore_con_cb_udp_listen(void *data, Ecore_Con_Info *info);

static void      _ecore_con_server_free(Ecore_Con_Server *svr);
static void      _ecore_con_client_free(Ecore_Con_Client *cl);

static Eina_Bool _ecore_con_svr_tcp_handler(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_cl_handler(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_cl_udp_handler(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_svr_udp_handler(void *data, Ecore_Fd_Handler *fd_handler);

static void      _ecore_con_svr_cl_read(Ecore_Con_Client *cl);
static Eina_Bool _ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler);

static void      _ecore_con_server_flush(Ecore_Con_Server *svr);
static void      _ecore_con_client_flush(Ecore_Con_Client *cl);

static void      _ecore_con_event_client_add_free(void *data, void *ev);
static void      _ecore_con_event_client_del_free(void *data, void *ev);
static void      _ecore_con_event_client_data_free(void *data, void *ev);
static void      _ecore_con_event_server_add_free(void *data, void *ev);
static void      _ecore_con_event_server_del_free(void *data, void *ev);
static void      _ecore_con_event_server_data_free(void *data, void *ev);

static void      _ecore_con_lookup_done(void *data, Ecore_Con_Info *infos);

EAPI int ECORE_CON_EVENT_CLIENT_ADD = 0;
EAPI int ECORE_CON_EVENT_CLIENT_DEL = 0;
EAPI int ECORE_CON_EVENT_SERVER_ADD = 0;
EAPI int ECORE_CON_EVENT_SERVER_DEL = 0;
EAPI int ECORE_CON_EVENT_CLIENT_DATA = 0;
EAPI int ECORE_CON_EVENT_SERVER_DATA = 0;

static Eina_List *servers = NULL;
static int _ecore_con_init_count = 0;
int _ecore_con_log_dom = -1;

/**
 * @addtogroup Ecore_Con_Lib_Group Ecore Connection Library Functions
 *
 * Utility functions that set up and shut down the Ecore Connection
 * library.
 * @{
 */

/**
 * Initialises the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int
ecore_con_init(void)
{
   if (++_ecore_con_init_count != 1)
      return _ecore_con_init_count;

#ifdef HAVE_EVIL
   if (!evil_init())
      return --_ecore_con_init_count;

#endif

   if (!ecore_init())
      return --_ecore_con_init_count;

   _ecore_con_log_dom = eina_log_domain_register("EcoreCon",
                                                 ECORE_CON_DEFAULT_LOG_COLOR);
   if(_ecore_con_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for Ecore Con.");
        ecore_shutdown();
        return --_ecore_con_init_count;
     }

   ECORE_CON_EVENT_CLIENT_ADD = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_DEL = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_DATA = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_DATA = ecore_event_type_new();

   /* TODO Remember return value, if it fails, use gethostbyname() */
   ecore_con_ssl_init();
   ecore_con_info_init();

   return _ecore_con_init_count;
}

/**
 * Shuts down the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int
ecore_con_shutdown(void)
{
   if (--_ecore_con_init_count != 0)
      return _ecore_con_init_count;

   while (servers)
      _ecore_con_server_free(eina_list_data_get(servers));

   ecore_con_info_shutdown();
   ecore_con_ssl_shutdown();
   eina_log_domain_unregister(_ecore_con_log_dom);
   _ecore_con_log_dom = -1;
   ecore_shutdown();
#ifdef HAVE_EVIL
   evil_shutdown();
#endif

   return _ecore_con_init_count;
}

/**
 * Do an asynchronous DNS lookup.
 *
 * This function performs a DNS lookup on the hostname specified by @p name, then
 * calls @p done_cb with
 *
 * @param name IP address or server name to translate.
 * @param done_cb Callback to notify when done.
 * @param data User data to be given to done_cb.
 * @return EINA_TRUE if the request did not fail to be set up, EINA_FALSE if it failed.
 */
EAPI Eina_Bool
ecore_con_lookup(const char *name, Ecore_Con_Dns_Cb done_cb, const void *data)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Lookup *lk;
   struct addrinfo hints;

   if (!name || !done_cb)
      return EINA_FALSE;

   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr)
      return EINA_FALSE;

   lk = malloc(sizeof (Ecore_Con_Lookup));
   if (!lk)
     {
        free(svr);
        return EINA_FALSE;
     }

   lk->done_cb = done_cb;
   lk->data = data;

   svr->name = strdup(name);
   if (!svr->name)
      goto on_error;

   svr->type = ECORE_CON_REMOTE_TCP;
   svr->port = 1025;
   svr->data = lk;
   svr->created = EINA_TRUE;
   svr->reject_excess_clients = EINA_FALSE;
   svr->client_limit = -1;
   svr->clients = NULL;
   svr->ppid = getpid();

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET6;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   if (ecore_con_info_get(svr, _ecore_con_lookup_done, svr,
                          &hints))
      return EINA_TRUE;

   free(svr->name);
on_error:
   free(lk);
   free(svr);
   return EINA_FALSE;
}

/**
 * @}
 */


/**
 * @addtogroup Ecore_Con_Server_Group Ecore Connection Server Functions
 *
 * Functions that operate on Ecore server objects.
 *
 * @{
 */

/**
 * @example ecore_con_server_example.c
 * Shows how to write a simple server using the Ecore_Con library.
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
 * @li If @a compl_type is @c ECORE_CON_REMOTE_TCP, the server will listen
 *     on TCP port @c port.
 *
 * @param  compl_type The connection type.
 * @param  name       Name to associate with the socket.  It is used when
 *                    generating the socket name of a Unix socket, or for
 *                    determining what host to listen on for TCP sockets.
 *                    @c NULL will not be accepted.
 * @param  port       Number to identify socket.  When a Unix socket is used,
 *                    it becomes part of the socket name.  When a TCP socket
 *                    is used, it is used as the TCP port.
 * @param  data       Data to associate with the created Ecore_Con_Server
 *                    object.
 * @return A new Ecore_Con_Server.
 */
EAPI Ecore_Con_Server *
ecore_con_server_add(Ecore_Con_Type compl_type,
                     const char *name,
                     int port,
                     const void *data)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;

   if (port < 0 || !name)
      return NULL;  /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */

   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr)
      return NULL;

   svr->name = strdup(name);
   if (!svr->name)
      goto error;

   svr->type = compl_type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = EINA_TRUE;
   svr->use_cert = (compl_type & ECORE_CON_LOAD_CERT);
   svr->reject_excess_clients = EINA_FALSE;
   svr->client_limit = -1;
   svr->clients = NULL;
   svr->ppid = getpid();
   if (ecore_con_ssl_server_prepare(svr, compl_type & ECORE_CON_SSL))
     goto error;

   type = compl_type & ECORE_CON_TYPE;

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM) ||
       (type == ECORE_CON_LOCAL_ABSTRACT))
      /* Local */
      if (!ecore_con_local_listen(svr, _ecore_con_svr_tcp_handler, svr))
         goto error;

   if ((type == ECORE_CON_REMOTE_TCP) ||
       (type == ECORE_CON_REMOTE_NODELAY))
     {
        /* TCP */
        if (!ecore_con_info_tcp_listen(svr, _ecore_con_cb_tcp_listen,
                                       svr))
           goto error;
     }
   else if ((type == ECORE_CON_REMOTE_MCAST) ||
            (type == ECORE_CON_REMOTE_UDP))
      /* UDP and MCAST */
      if (!ecore_con_info_udp_listen(svr, _ecore_con_cb_udp_listen,
                                     svr))
         goto error;

   servers = eina_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);

   return svr;

error:
   if (svr->name)
      free(svr->name);

   if (svr->path)
      free(svr->path);

#ifndef _WIN32
   if (svr->fd >= 0)
      close(svr->fd);

   if (svr->fd_handler)
      ecore_main_fd_handler_del(svr->fd_handler);

   if (svr->write_buf)
      free(svr->write_buf);

   if (svr->ip)
      free(svr->ip);

#endif
      ecore_con_ssl_server_shutdown(svr);
   free(svr);
   return NULL;
}

/**
 * Creates a connection to the specified server and returns an associated object.
 *
 * The socket to which the connection is made depends on the connection type:
 * @li If @a compl_type is @c ECORE_CON_LOCAL_USER, the function will
 *     connect to the server at the Unix socket
 *     "~/.ecore/[name]/[port]".
 * @li If @a compl_type is @c ECORE_CON_LOCAL_SYSTEM, the function will
 *     connect to the server at the Unix socket
 *     "/tmp/.ecore_service|[name]|[port]".
 * @li If @a compl_type is @c ECORE_CON_REMOTE_TCP, the function will
 *     connect to the server at the TCP port "[name]:[port]".
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
 */
EAPI Ecore_Con_Server *
ecore_con_server_connect(Ecore_Con_Type compl_type,
                         const char *name,
                         int port,
                         const void *data)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;

   if (!name)
      return NULL;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr)
      return NULL;

   svr->name = strdup(name);
   if (!svr->name)
      goto error;

   svr->type = compl_type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = EINA_FALSE;
   svr->use_cert = (compl_type & ECORE_CON_LOAD_CERT);
   svr->reject_excess_clients = EINA_FALSE;
   svr->clients = NULL;
   svr->client_limit = -1;
   if (ecore_con_ssl_server_prepare(svr, compl_type & ECORE_CON_SSL))
     goto error;

   type = compl_type & ECORE_CON_TYPE;

   if (((type == ECORE_CON_REMOTE_TCP) ||
        (type == ECORE_CON_REMOTE_NODELAY) ||
        (type == ECORE_CON_REMOTE_UDP) ||
        (type == ECORE_CON_REMOTE_BROADCAST)) &&
       (port < 0))
      goto error;

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM) ||
       (type == ECORE_CON_LOCAL_ABSTRACT))
      /* Local */
      if (!ecore_con_local_connect(svr, _ecore_con_cl_handler, svr,
                                   _ecore_con_event_server_add_free))
         goto
         error;

   if ((type == ECORE_CON_REMOTE_TCP) ||
       (type == ECORE_CON_REMOTE_NODELAY))
     {
        /* TCP */
        if (!ecore_con_info_tcp_connect(svr, _ecore_con_cb_tcp_connect,
                                        svr))
           goto error;
     }
   else if ((type == ECORE_CON_REMOTE_UDP) ||
            (type == ECORE_CON_REMOTE_BROADCAST))
      /* UDP and MCAST */
      if (!ecore_con_info_udp_connect(svr, _ecore_con_cb_udp_connect,
                                      svr))
         goto error;

   servers = eina_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);

   return svr;

error:
   if (svr->name)
      free(svr->name);

   if (svr->path)
      free(svr->path);

   if (svr->fd >= 0)
      close(svr->fd);

   if (svr->fd_handler)
      ecore_main_fd_handler_del(svr->fd_handler);

   ecore_con_ssl_server_shutdown(svr);
   free(svr);
   return NULL;
}

/**
 * Set the default time after which an inactive client will be disconnected
 * @param svr The server object
 * @param timeout The timeout, in seconds, to disconnect after
 * This function is used to set the idle timeout on clients.  A value of < 1
 * disables the idle timeout.
 */
EAPI void
ecore_con_server_timeout_set(Ecore_Con_Server *svr, double timeout)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_timeout_set");
        return;
     }

   svr->client_disconnect_time = timeout;
}

/**
 * Get the default time after which an inactive client will be disconnected
 * @param svr The server object
 * @return The timeout, in seconds, to disconnect after
 * This function is used to get the idle timeout for clients.  A value of < 1
 * means the idle timeout is disabled.
 */
EAPI double
ecore_con_server_timeout_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_timeout_get");
        return 0;
     }

   return svr->client_disconnect_time;
}

/**
 * Closes the connection and frees the given server.
 * @param   svr The given server.
 * @return  Data associated with the server when it was created.
 * @see ecore_con_server_add, ecore_con_server_connect
 */
EAPI void *
ecore_con_server_del(Ecore_Con_Server *svr)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_del");
        return NULL;
     }

   if (svr->delete_me)
      return NULL;

   data = svr->data;
   svr->data = NULL;
   svr->delete_me = EINA_TRUE;
   if (svr->event_count > 0)
     {
        if (svr->fd_handler)
          {
             ecore_main_fd_handler_del(svr->fd_handler);
             svr->fd_handler = NULL;
          }
     }
   else
      _ecore_con_server_free(svr);

   return data;
}

/**
 * Retrieves the data associated with the given server.
 * @param   svr The given server.
 * @return  The associated data.
 */
EAPI void *
ecore_con_server_data_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr,
                         ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_data_get");
        return NULL;
     }

   return svr->data;
}

/**
 * Sets the data associated with the given server.
 * @param svr The given server.
 * @param data The data to associate with @p svr
 * @return  The previously associated data, if any.
 */
EAPI void *
ecore_con_server_data_set(Ecore_Con_Server *svr, void *data)
{
   void *ret = NULL;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr,
                         ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_data_get");
        return NULL;
     }

   ret = svr->data;
   svr->data = data;
   return ret;
}

/**
 * Retrieves whether the given server is currently connected.
 * @param   svr The given server.
 * @return  #EINA_TRUE if the server is connected.  #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_con_server_connected_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_connected_get");
        return EINA_FALSE;
     }

   if (svr->connecting)
      return EINA_FALSE;

   return EINA_TRUE;
}

/**
 * Retrieves the current list of clients.
 * @param   svr The given server.
 * @return  The list of clients on this server.
 */
EAPI Eina_List *
ecore_con_server_clients_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_clients_get");
        return NULL;
     }

   return svr->clients;
}

/**
 * Retrieves the name of server.
 * @param   svr The given server.
 * @return  The name of the server.
 */
EAPI const char *
ecore_con_server_name_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_name_get");
        return NULL;
     }

   return svr->name;
}

/**
 * Retrieves the server port in use.
 * @param   svr The given server.
 * @return  The server port in use.
 */
EAPI int
ecore_con_server_port_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_port_get");
        return -1;
     }
   return svr->port;
}

/**
 * Sends the given data to the given server.
 * @param   svr  The given server.
 * @param   data The given data.
 * @param   size Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is an
 *          error.
 */
EAPI int
ecore_con_server_send(Ecore_Con_Server *svr,
                      const void *data,
                      int size)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_send");
        return 0;
     }

   if (svr->dead)
      return 0;

   if (!data)
      return 0;

   if (size < 1)
      return 0;

   if (svr->fd_handler)
      ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);

   if (svr->write_buf)
     {
        unsigned char *newbuf;

        newbuf = realloc(svr->write_buf, svr->write_buf_size + size);
        if (newbuf)
           svr->write_buf = newbuf;
        else
           return 0;

        memcpy(svr->write_buf + svr->write_buf_size, data, size);
        svr->write_buf_size += size;
     }
   else
     {
        svr->write_buf = malloc(size);
        if (!svr->write_buf)
           return 0;

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
 */
EAPI void
ecore_con_server_client_limit_set(Ecore_Con_Server *svr,
                                  int client_limit,
                                  char reject_excess_clients)
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
 * Gets the IP address of a server that has been connected to.
 *
 * @param   svr           The given server.
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected server in the form "XXX.YYY.ZZZ.AAA" IP notation.
 *          This string should not be modified or trusted to stay valid after
 *          deletion for the @p svr object. If no IP is known NULL is returned.
 */
EAPI const char *
ecore_con_server_ip_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_ip_get");
        return NULL;
     }

   return svr->ip;
}


/**
 * @brief Check how long a server has been connected
 * @param svr The server to check
 * @return The total time, in seconds, that the server has been connected/running
 * This function is used to find out how long a server has been connected/running for.
 */
EAPI double
ecore_con_server_uptime_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_uptime_get");
        return -1;
     }

   return ecore_time_get() - svr->start_time;
}
/**
 * Flushes all pending data to the given server. Will return when done.
 *
 * @param   svr           The given server.
 */
EAPI void
ecore_con_server_flush(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_flush");
        return;
     }

   _ecore_con_server_flush(svr);
}


/**
 * @}
 */

/**
 * @addtogroup Ecore_Con_Client_Group Ecore Connection Client Functions
 *
 * Functions that operate on Ecore connection client objects.
 *
 * @{
 */

/**
 * @example ecore_con_client_example.c
 * Shows how to write a simple client that connects to the example server.
*/

/**
 * Sends the given data to the given client.
 * @param   cl   The given client.
 * @param   data The given data.
 * @param   size Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is an
 *          error.
 */
EAPI int
ecore_con_client_send(Ecore_Con_Client *cl,
                      const void *data,
                      int size)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_send");
        return 0;
     }

   if (cl->dead)
      return 0;

   if (!data)
      return 0;

   if (size < 1)
      return 0;

   if (cl->fd_handler)
      ecore_main_fd_handler_active_set(
         cl->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);

   if(cl->host_server && ((cl->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP))
      sendto(cl->host_server->fd, data, size, 0, (struct sockaddr *)cl->client_addr,
             cl->client_addr_len);
   else if (cl->buf)
     {
        unsigned char *newbuf;

        newbuf = realloc(cl->buf, cl->buf_size + size);
        if (newbuf)
           cl->buf = newbuf;
        else
           return 0;

        memcpy(cl->buf + cl->buf_size, data, size);
        cl->buf_size += size;
     }
   else
     {
        cl->buf = malloc(size);
        if (!cl->buf)
           return 0;

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
 */
EAPI Ecore_Con_Server *
ecore_con_client_server_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_server_get");
        return NULL;
     }

   return cl->host_server;
}

/**
 * Set the time after which the client will be disconnected when inactive
 * @param cl The client object
 * @param timeout The timeout, in seconds, to disconnect after
 * This function is used to set the idle timeout on a client.  A value of < 1
 * disables the idle timeout.
 */
EAPI void
ecore_con_client_timeout_set(Ecore_Con_Client *cl, double timeout)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_timeout_set");
        return;
     }

   cl->disconnect_time = timeout;

   _ecore_con_cl_timer_update(cl);
}

/**
 * Get the default time after which the client will be disconnected when inactive
 * @param cl The client object
 * @return The timeout, in seconds, to disconnect after
 * This function is used to get the idle timeout for a client.  A value of < 1
 * means the idle timeout is disabled.
 */
EAPI double
ecore_con_client_timeout_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_timeout_get");
        return 0;
     }

   return cl->disconnect_time;
}

/**
 * Closes the connection and frees memory allocated to the given client.
 * @param   cl The given client.
 * @return  Data associated with the client.
 */
EAPI void *
ecore_con_client_del(Ecore_Con_Client *cl)
{
   void *data = NULL;

   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_del");
        return NULL;
     }

   if (cl->client_addr && cl->host_server &&
      (((cl->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP) ||
       ((cl->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST)))
      free(cl->client_addr);

   data = cl->data;

   cl->data = NULL;
   cl->delete_me = EINA_TRUE;
   if (cl->event_count > 0)
     {
        if (cl->fd_handler)
          {
             ecore_main_fd_handler_del(cl->fd_handler);
             cl->fd_handler = NULL;
          }
     }
   else
     {
        if (cl->host_server)
          {
             cl->host_server->clients = eina_list_remove(cl->host_server->clients, cl);
             --cl->host_server->client_count;
          }

        _ecore_con_client_free(cl);
     }

   return data;
}

/**
 * Sets the data associated with the given client to @p data.
 * @param   cl   The given client.
 * @param   data What to set the data to.
 */
EAPI void
ecore_con_client_data_set(Ecore_Con_Client *cl, const void *data)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl,
                         ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_data_set");
        return;
     }

   cl->data = (void *)data;
}

/**
 * Retrieves the data associated with the given client.
 * @param   cl The given client.
 * @return  The data associated with @p cl.
 */
EAPI void *
ecore_con_client_data_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl,
                         ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_data_get");
        return NULL;
     }

   return cl->data;
}

/**
 * Gets the IP address of a cleint that has connected.
 *
 * @param   cl            The given client.
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected client in the form "XXX.YYY.ZZZ.AAA" IP notation.
 *          This string should not be modified or trusted to stay valid after
 *          deletion for the @p cl object. If no IP is known NULL is returned.
 */
EAPI const char *
ecore_con_client_ip_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_ip_get");
        return NULL;
     }

   return cl->ip;
}

/**
 * @brief Check how long a client has been connected
 * @param cl The client to check
 * @return The total time, in seconds, that the client has been connected to the server
 * This function is used to find out how long a client has been connected for.
 */
EAPI double
ecore_con_client_uptime_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_uptime_get");
        return -1;
     }

   return ecore_time_get() - cl->start_time;
}

/**
 * Flushes all pending data to the given client. Will return when done.
 *
 * @param   cl            The given client.
 */
EAPI void
ecore_con_client_flush(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_flush");
        return;
     }

   _ecore_con_client_flush(cl);
}

/**
 * @}
 */

static void
_ecore_con_server_free(Ecore_Con_Server *svr)
{
   Ecore_Con_Client *cl;
   double t_start, t;

   ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);
   t_start = ecore_time_get();
   while ((svr->write_buf) && (!svr->dead))
     {
        _ecore_con_server_flush(svr);
        t = ecore_time_get();
        if ((t - t_start) > 0.5)
          {
             WRN("ECORE_CON: EEK - stuck in _ecore_con_server_free() trying\n"
                 "  to flush data out from the server, and have been for\n"
                 "  %1.1f seconds. This is taking too long. Aborting flush.",
                 (t - t_start));
             break;
          }
     }

   if (svr->write_buf)
      free(svr->write_buf);

   EINA_LIST_FREE(svr->clients, cl)
     _ecore_con_client_free(cl);
   if ((svr->created) && (svr->path) && (svr->ppid == getpid()))
      unlink(svr->path);

   ecore_con_ssl_server_shutdown(svr);
   if (svr->fd >= 0)
      close(svr->fd);

   if (svr->name)
      free(svr->name);

   if (svr->path)
      free(svr->path);

   if (svr->ip)
      free(svr->ip);

   if (svr->fd_handler)
      ecore_main_fd_handler_del(svr->fd_handler);

   servers = eina_list_remove(servers, svr);
   free(svr);
}

static void
_ecore_con_client_free(Ecore_Con_Client *cl)
{
   double t_start, t;

   if ((!cl->buf) && cl->delete_me && (!cl->dead) && (cl->event_count < 1))
     {
      /* this is a catch-all for cases when a client is not properly killed.
       * the only example case I've found so far is if a client ssl handshakes
       * and then immediately disconnects without sending any further data.
       */

        /* we lost our client! */
        Ecore_Con_Event_Client_Del *e;

        cl->dead = EINA_TRUE;
        e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
        if (e)
          {
             cl->event_count++;
             _ecore_con_cl_timer_update(cl);
             e->client = cl;
             ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                             _ecore_con_event_client_del_free, NULL);
             return;
          }
     }

   ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);
   t_start = ecore_time_get();
   while ((cl->buf) && (!cl->dead))
     {
        _ecore_con_client_flush(cl);
        t = ecore_time_get();
        if ((t - t_start) > 0.5)
          {
             WRN("EEK - stuck in _ecore_con_client_free() trying\n"
                 "  to flush data out from the client, and have been for\n"
                 "  %1.1f seconds. This is taking too long. Aborting flush.",
                 (t - t_start));
             break;
          }
     }
   if (cl->buf)
      free(cl->buf);

   if (cl->host_server->type & ECORE_CON_SSL)
     ecore_con_ssl_client_shutdown(cl);

   if (cl->fd >= 0)
      close(cl->fd);

   if (cl->fd_handler)
      ecore_main_fd_handler_del(cl->fd_handler);

   if (cl->ip)
      free(cl->ip);

      free(cl);
   return;
}

static void
_ecore_con_server_kill(Ecore_Con_Server *svr)
{
   if (!svr->delete_me)
     {
        Ecore_Con_Event_Server_Del *e;

        e = calloc(1, sizeof(Ecore_Con_Event_Server_Del));
        if (e)
          {
             svr->event_count++;
             e->server = svr;
             ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e,
                             _ecore_con_event_server_del_free, NULL);
          }
     }

   svr->dead = EINA_TRUE;
   if (svr->fd_handler)
      ecore_main_fd_handler_del(svr->fd_handler);

   svr->fd_handler = NULL;
}

static Eina_Bool
_ecore_con_client_timer(Ecore_Con_Client *cl)
{
   ecore_con_client_del(cl);

   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_cl_timer_update(Ecore_Con_Client *cl)
{
   if (cl->disconnect_time)
     {
        if (cl->disconnect_time > 0)
          {
             if (cl->until_deletion)
               ecore_timer_interval_set(cl->until_deletion, cl->disconnect_time);
             else
               cl->until_deletion = ecore_timer_add(cl->disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, cl);
          }
        else if (cl->until_deletion)
          {
             ecore_timer_del(cl->until_deletion);
             cl->until_deletion = NULL;
          }
     }
   else
     {
        if (cl->host_server->client_disconnect_time > 0)
          {
             if (cl->until_deletion)
               ecore_timer_interval_set(cl->until_deletion, cl->host_server->client_disconnect_time);
             else
               cl->until_deletion = ecore_timer_add(cl->host_server->client_disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, cl);
          }
        else if (cl->until_deletion)
          {
             ecore_timer_del(cl->until_deletion);
             cl->until_deletion = NULL;
          }
     }
}

static void
_ecore_con_cb_tcp_listen(void *data, Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   struct linger lin;

   svr = data;

   if(!net_info)
      goto error;

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
      goto error;

   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, (const void *)&lin,
                  sizeof(struct linger)) < 0)
      goto error;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_NODELAY)
     {
        int flag = 1;

        if (setsockopt(svr->fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,
                       sizeof(int)) < 0)
           goto error;
     }

   if (bind(svr->fd, net_info->info.ai_addr,
            net_info->info.ai_addrlen) < 0)
      goto error;

   if (listen(svr->fd, 4096) < 0)
      goto error;

   svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                _ecore_con_svr_tcp_handler, svr, NULL, NULL);
   if (!svr->fd_handler)
      goto error;

   return;

error:
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static void
_ecore_con_cb_udp_listen(void *data, Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;
   struct ip_mreq mreq;
   struct ipv6_mreq mreq6;
   const int on = 1;

   svr = data;
   type = svr->type;
   type &= ECORE_CON_TYPE;

   if (!net_info)
      goto error;

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if(svr->fd < 0)
      goto error;

   if (type == ECORE_CON_REMOTE_MCAST)
     {
        if (net_info->info.ai_family == AF_INET)
          {
             if (!inet_pton(net_info->info.ai_family, net_info->ip,
                            &mreq.imr_multiaddr))
                goto error;

             mreq.imr_interface.s_addr = htonl(INADDR_ANY);
             if (setsockopt(svr->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                            (const void *)&mreq,sizeof(mreq)) != 0)
                goto error;
          }
        else if (net_info->info.ai_family == AF_INET6)
          {
             if (!inet_pton(net_info->info.ai_family, net_info->ip,
                            &mreq6.ipv6mr_multiaddr))
                goto error;

             mreq6.ipv6mr_interface = htonl(INADDR_ANY);
             if (setsockopt(svr->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                            (const void *)&mreq6,sizeof(mreq6)) != 0)
                goto error;
          }

        if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&on,
                       sizeof(on)) != 0)
           goto error;
     }

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
      goto error;

   if (bind(svr->fd, net_info->info.ai_addr,
            net_info->info.ai_addrlen) < 0)
      goto error;

   svr->fd_handler =
      ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                _ecore_con_svr_udp_handler, svr, NULL, NULL);
   if (!svr->fd_handler)
      goto error;

   svr->ip = strdup(net_info->ip);

   return;

error:
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static void
_ecore_con_cb_tcp_connect(void *data, Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   int res;
   int curstate = 0;

   svr = data;

   if (!net_info)
      goto error;

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
      goto error;

   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate,
                  sizeof(curstate)) < 0)
      goto error;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_NODELAY)
     {
        int flag = 1;

        if (setsockopt(svr->fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,
                       sizeof(int)) < 0)
           goto error;
     }

   res = connect(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen);
#ifdef _WIN32
   if (res == SOCKET_ERROR)
     {
        if (WSAGetLastError() != WSAEINPROGRESS)
           goto error;

#else
   if (res < 0)
     {
        if (errno != EINPROGRESS)
           goto error;

#endif
        svr->connecting = EINA_TRUE;
        svr->fd_handler =
           ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ | ECORE_FD_WRITE,
                                     _ecore_con_cl_handler, svr, NULL, NULL);
     }
   else
      svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                   _ecore_con_cl_handler, svr, NULL, NULL);

   if (svr->type & ECORE_CON_SSL)
     {
        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_INIT;
        DBG("beginning ssl handshake");
        if (ecore_con_ssl_server_init(svr))
          goto error;
     }

   if (!svr->fd_handler)
      goto error;

   svr->ip = strdup(net_info->ip);

   return;

error:
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static void
_ecore_con_cb_udp_connect(void *data, Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   int curstate = 0;
   int broadcast = 1;
   svr = data;

   if (!net_info)
      goto error;

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
      goto error;

   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
      goto error;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_BROADCAST)
     {
        if (setsockopt(svr->fd, SOL_SOCKET, SO_BROADCAST,
                       (const void *)&broadcast,
                       sizeof(broadcast)) < 0)
           goto error;
     }
   else if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR,
                       (const void *)&curstate, sizeof(curstate)) < 0)
      goto error;

   if (connect(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen) < 0)
      goto error;

   svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ | ECORE_FD_WRITE,
                                   _ecore_con_cl_udp_handler, svr, NULL, NULL);

   if (!svr->fd_handler)
      goto error;

   svr->ip = strdup(net_info->ip);

   return;

error:
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static Ecore_Con_State
svr_try_connect_plain(Ecore_Con_Server *svr)
{
   int res;
   int so_err = 0;
   unsigned int size = sizeof(int);

   res = getsockopt(svr->fd, SOL_SOCKET, SO_ERROR, (void *)&so_err, &size);
#ifdef _WIN32
   if (res == SOCKET_ERROR)
      so_err = -1;

   if (so_err == WSAEINPROGRESS && !svr->dead)
      return ECORE_CON_INPROGRESS;

#else
   if (res < 0)
      so_err = -1;

   if (so_err == EINPROGRESS && !svr->dead)
      return ECORE_CON_INPROGRESS;

#endif

   if (so_err != 0)
     {
        /* we lost our server! */
        _ecore_con_server_kill(svr);
        return ECORE_CON_DISCONNECTED;
     }

   if ((!svr->delete_me) && (!svr->handshaking) && svr->connecting)
     {
        /* we got our server! */
        Ecore_Con_Event_Server_Add *e;

        svr->connecting = EINA_FALSE;
        e = calloc(1, sizeof(Ecore_Con_Event_Server_Add));
        if (e)
          {
             svr->event_count++;
             svr->start_time = ecore_time_get();
             e->server = svr;
             ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, e,
                             _ecore_con_event_server_add_free, NULL);
          }
     }

   if (svr->fd_handler && (!svr->write_buf))
         ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);

   if (!svr->dead)
      return ECORE_CON_CONNECTED;
   else
      return ECORE_CON_DISCONNECTED;
}

/* returns 1 on success, 0 on failure */
static Ecore_Con_State svr_try_connect(Ecore_Con_Server *svr)
{
   if (!(svr->type & ECORE_CON_SSL))
      return svr_try_connect_plain(svr);

   switch (ecore_con_ssl_server_try(svr))
     {
      case ECORE_CON_CONNECTED:
         return svr_try_connect_plain(svr);

      case ECORE_CON_DISCONNECTED:
         _ecore_con_server_kill(svr);
         return ECORE_CON_DISCONNECTED;

      default:
         return ECORE_CON_INPROGRESS;
     }
}

static char *
_ecore_con_pretty_ip(struct sockaddr *client_addr, socklen_t size)
{
   char ipbuf[INET6_ADDRSTRLEN + 1];

   /* show v4mapped address in pretty form */
   if (client_addr->sa_family == AF_INET6)
     {
        struct sockaddr_in6 *sa6;

        sa6 = (struct sockaddr_in6 *)client_addr;
        if (IN6_IS_ADDR_V4MAPPED(&sa6->sin6_addr))
          {
             snprintf(ipbuf, sizeof (ipbuf), "%u.%u.%u.%u",
                      sa6->sin6_addr.s6_addr[12],
                      sa6->sin6_addr.s6_addr[13],
                      sa6->sin6_addr.s6_addr[14],
                      sa6->sin6_addr.s6_addr[15]);
             return strdup(ipbuf);
          }
     }

   if (getnameinfo(client_addr, size,
                   ipbuf, sizeof (ipbuf), NULL, 0,
                   NI_NUMERICHOST))
      return strdup("0.0.0.0");

   ipbuf[sizeof (ipbuf) - 1] = 0;
   return strdup(ipbuf);
}

static Eina_Bool
_ecore_con_svr_tcp_handler(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Client *cl = NULL;
   int new_fd;
   unsigned char incoming[256];
   size_t size_in;

   svr = data;
   if (svr->dead)
      return ECORE_CALLBACK_RENEW;

   if (svr->delete_me)
      return ECORE_CALLBACK_RENEW;

   if ((svr->client_limit >= 0) && (!svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     return ECORE_CALLBACK_RENEW;

   /* a new client */

   size_in = sizeof(incoming);

   memset(&incoming, 0, size_in);
   new_fd = accept(svr->fd, (struct sockaddr *)&incoming, (socklen_t *)&size_in);
   if (new_fd < 0)
     /* error! */
     return ECORE_CALLBACK_RENEW;

   if ((svr->client_limit >= 0) && (svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     goto error;

   cl = calloc(1, sizeof(Ecore_Con_Client));
   if (!cl)
     goto error;

   fcntl(new_fd, F_SETFL, O_NONBLOCK);
   fcntl(new_fd, F_SETFD, FD_CLOEXEC);
   cl->fd = new_fd;
   cl->host_server = svr;

   if (svr->type & ECORE_CON_SSL)
     {
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_INIT;
        if (ecore_con_ssl_client_init(cl))
          goto error;
     }

   cl->fd_handler = ecore_main_fd_handler_add(cl->fd, ECORE_FD_READ,
                                _ecore_con_svr_cl_handler, cl, NULL, NULL);
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);
   svr->clients = eina_list_append(svr->clients, cl);
   svr->client_count++;
   if (!svr->path)
      cl->ip = _ecore_con_pretty_ip((struct sockaddr *)&incoming, size_in);

   if ((!cl->delete_me) && (!cl->handshaking))
     {
        Ecore_Con_Event_Client_Add *e;

        e = calloc(1, sizeof(Ecore_Con_Event_Client_Add));
        if (e)
          {
             cl->event_count++;
             _ecore_con_cl_timer_update(cl);
             e->client = cl;
             ecore_event_add(ECORE_CON_EVENT_CLIENT_ADD, e,
                             _ecore_con_event_client_add_free, NULL);
          }
     }

   return ECORE_CALLBACK_RENEW;

error:
   close(new_fd);
   return ECORE_CALLBACK_RENEW;
}


static void
_ecore_con_cl_read(Ecore_Con_Server *svr)
{
   unsigned char *inbuf = NULL;
   int inbuf_num = 0;
   int tries;

   if (svr->connecting && (svr_try_connect(svr) != ECORE_CON_CONNECTED))
      return;

   for (tries = 0; tries < 16; tries++)
     {
        int num;
        Eina_Bool lost_server = EINA_TRUE;
        unsigned char buf[READBUFSIZ];

        if (svr->handshaking)
          {
             DBG("Continuing ssl handshake");
             if (!ecore_con_ssl_server_init(svr))
               lost_server = EINA_FALSE;
          }

        if (!(svr->type & ECORE_CON_SSL))
          {
             num = read(svr->fd, buf, READBUFSIZ);
             if ((num < 0) && (errno == EAGAIN))
               lost_server = EINA_FALSE;
          }
        else if (!(num = ecore_con_ssl_server_read(svr, buf, READBUFSIZ)))
           lost_server = EINA_FALSE;

        if (num < 1)
          {
             if (inbuf && (!svr->delete_me))
               {
                  Ecore_Con_Event_Server_Data *e;

                  e = malloc(sizeof(Ecore_Con_Event_Server_Data));
                  if (e)
                    {
                       svr->event_count++;
                       e->server = svr;
                       e->data = inbuf;
                       e->size = inbuf_num;
                       ecore_event_add(ECORE_CON_EVENT_SERVER_DATA, e,
                                       _ecore_con_event_server_data_free, NULL);
                    }
               }

             if (lost_server)
                _ecore_con_server_kill(svr);

             break;
          }

        inbuf = realloc(inbuf, inbuf_num + num);
        memcpy(inbuf + inbuf_num, buf, num);
        inbuf_num += num;
     }

/* #if USE_OPENSSL */
/* if (svr->fd_handler) */
/*   { */
/*      if (svr->ssl && ssl_err == SSL_ERROR_WANT_READ) */
/*        ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ); */
/*      else if (svr->ssl && ssl_err == SSL_ERROR_WANT_WRITE) */
/*        ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE); */
/*   } */
/* #endif */
}

static Eina_Bool
_ecore_con_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server *svr;
   Eina_Bool want_read, want_write;

   svr = data;
   if (svr->dead)
      return ECORE_CALLBACK_RENEW;

   if (svr->delete_me)
      return ECORE_CALLBACK_RENEW;

   want_read = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ);
   want_write = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE);

   if (svr->handshaking && (want_read || want_write))
     {
        DBG("Continuing ssl handshake: preparing to %s...", want_read ? "read" : "write");
#ifdef PRINT_LOTS_OF_DEBUG
             char buf[32768];
             ssize_t len;
             len = recv(svr->fd, buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK);
             DBG("%zu bytes in buffer", len);
#endif
        if (!svr->ssl_state)
          {
             /* we got our server! */
             Ecore_Con_Event_Server_Add *e;

             svr->connecting = EINA_FALSE;
             e = calloc(1, sizeof(Ecore_Con_Event_Server_Add));
             if (e)
               {
                  svr->event_count++;
                  svr->start_time = ecore_time_get();
                  e->server = svr;
                  ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, e,
                                  _ecore_con_event_server_add_free, NULL);
               }
          }
        else if (ecore_con_ssl_server_init(svr))
          {
             ERR("ssl handshaking failed!");
             Ecore_Con_Event_Server_Del *e;

             e = calloc(1, sizeof(Ecore_Con_Event_Server_Del));
             if (e)
               {
                  svr->event_count++;
                  e->server = svr;
                  ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e,
                                  _ecore_con_event_server_del_free, NULL);
               }
          }
     }

   else if (want_read)
     _ecore_con_cl_read(svr);
   else if (want_write)
     {
        if (svr->connecting && (!svr_try_connect(svr)))
           return ECORE_CALLBACK_RENEW;

        _ecore_con_server_flush(svr);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_cl_udp_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server *svr;

   svr = data;
   if (svr->dead)
      return ECORE_CALLBACK_RENEW;

   if (svr->delete_me)
      return ECORE_CALLBACK_RENEW;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
        unsigned char buf[65536];
        int num = 0;

        errno = 0;
        num = read(svr->fd, buf, 65536);
        if (num > 0)
          {
             if (!svr->delete_me)
               {
                  Ecore_Con_Event_Server_Data *e;
                  unsigned char *inbuf;

                  inbuf = malloc(num);
                  if(!inbuf)
                     return 1;

                  memcpy(inbuf, buf, num);

                  e = calloc(1, sizeof(Ecore_Con_Event_Server_Data));
                  if (e)
                    {
                       svr->event_count++;
                       e->server = svr;
                       e->data = inbuf;
                       e->size = num;
                       ecore_event_add(ECORE_CON_EVENT_SERVER_DATA, e,
                                       _ecore_con_event_server_data_free,
                                       NULL);
                    }
               }
          }
        else if ((errno == EIO) || (errno == EBADF) ||
                 (errno == EPIPE) || (errno == EINVAL) ||
                 (errno == ENOSPC) || (errno == ECONNREFUSED))
           _ecore_con_server_kill(svr);
     }
   else if (ecore_main_fd_handler_active_get(fd_handler,
                                             ECORE_FD_WRITE))
      _ecore_con_server_flush(svr);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_svr_udp_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Client *cl = NULL;

   svr = data;
   if (svr->dead)
      return ECORE_CALLBACK_RENEW;

   if (svr->delete_me)
      return ECORE_CALLBACK_RENEW;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
        unsigned char buf[READBUFSIZ];
        unsigned char client_addr[256];
        unsigned int client_addr_len = sizeof(client_addr);
        int num;

        errno = 0;
#ifdef _WIN32
        num = fcntl(svr->fd, F_SETFL, O_NONBLOCK);
        if (num >= 0)
           num =
              recvfrom(svr->fd, buf, sizeof(buf), 0,
                       (struct sockaddr *)&client_addr,
                       &client_addr_len);

#else
        num =
              recvfrom(svr->fd, buf, sizeof(buf), MSG_DONTWAIT,
                    (struct sockaddr *)&client_addr,
                    &client_addr_len);
#endif

        if (num > 0)
          {
             if (!svr->delete_me)
               {
                  Ecore_Con_Event_Client_Data *e;
                  unsigned char *inbuf;

                  /* Create a new client for use in the client data event */
                  cl = calloc(1, sizeof(Ecore_Con_Client));
                  if(!cl)
                     return ECORE_CALLBACK_RENEW;

                  cl->buf = NULL;
                  cl->fd = 0;
                  cl->fd_handler = NULL;
                  cl->host_server = svr;
                  cl->client_addr = calloc(1, client_addr_len);
                  cl->client_addr_len = client_addr_len;
                  if(!cl->client_addr)
                    {
                       free(cl);
                       return ECORE_CALLBACK_RENEW;
                    }

                  memcpy(cl->client_addr, &client_addr, client_addr_len);
                  ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);
                  svr->clients = eina_list_append(svr->clients, cl);
                  svr->client_count++;

                  cl->ip = _ecore_con_pretty_ip(cl->client_addr,
                                                cl->client_addr_len);

                  inbuf = malloc(num);
                  if(!inbuf)
                    {
                       free(cl->client_addr);
                       free(cl);
                       return ECORE_CALLBACK_RENEW;
                    }

                  memcpy(inbuf, buf, num);

                  e = calloc(1, sizeof(Ecore_Con_Event_Client_Data));
                  if (e)
                    {
                       svr->event_count++;
                       _ecore_con_cl_timer_update(cl);
                       e->client = cl;
                       e->data = inbuf;
                       e->size = num;
                            ecore_event_add(ECORE_CON_EVENT_CLIENT_DATA, e,
                                       _ecore_con_event_client_data_free,
                                       NULL);
                    }

                  if (!cl->delete_me)
                    {
                       Ecore_Con_Event_Client_Add *add;

                       add = calloc(1, sizeof(Ecore_Con_Event_Client_Add));
                       if(add)
                         {
/*cl->event_count++;*/
                            add->client = cl;
                            _ecore_con_cl_timer_update(cl);
                            ecore_event_add(ECORE_CON_EVENT_CLIENT_ADD, add,
                                            _ecore_con_event_client_add_free, NULL);
                         }
                    }
               }
          }
        else if ((errno == EIO) || (errno == EBADF) ||
                 (errno == EPIPE) || (errno == EINVAL) ||
                 (errno == ENOSPC) || (errno == ECONNREFUSED))
          {
             if (!svr->delete_me)
               {
                  /* we lost our client! */
                  Ecore_Con_Event_Client_Del *e;

                  e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
                  if (e)
                    {
                       svr->event_count++;
                       /* be explicit here */
                       e->client = NULL;
                       ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                                  _ecore_con_event_client_del_free, NULL);
                    }
               }

             svr->dead = EINA_TRUE;
             if (svr->fd_handler)
                ecore_main_fd_handler_del(svr->fd_handler);

             svr->fd_handler = NULL;
          }
     }
   else if (ecore_main_fd_handler_active_get(fd_handler,
                                             ECORE_FD_WRITE))
      _ecore_con_client_flush(cl);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_svr_cl_read(Ecore_Con_Client *cl)
{
   unsigned char *inbuf = NULL;
   int inbuf_num = 0;
   int tries;

   for (tries = 0; tries < 16; tries++)
     {
        int num;
        Eina_Bool lost_client = EINA_TRUE;
        unsigned char buf[READBUFSIZ];

        errno = 0;

        if (cl->handshaking)
          {
             if (ecore_con_ssl_client_init(cl))
               lost_client = EINA_FALSE;

             _ecore_con_cl_timer_update(cl);
          }

        if (!(cl->host_server->type & ECORE_CON_SSL))
          {
             if ((num = read(cl->fd, buf, READBUFSIZ)) <= 0)
                if ((num < 0) && (errno == EAGAIN))
                   lost_client = EINA_FALSE;

          }
        else if (!(num = ecore_con_ssl_client_read(cl, buf, READBUFSIZ)))
           lost_client = EINA_FALSE;

        if (num > 0)
          {
             inbuf = realloc(inbuf, inbuf_num + num);
             memcpy(inbuf + inbuf_num, buf, num);
             inbuf_num += num;
             continue;
          }

        if (inbuf && (!cl->delete_me))
          {
             Ecore_Con_Event_Client_Data *e;

             e = malloc(sizeof(Ecore_Con_Event_Client_Data));
             if (e)
               {
                  cl->event_count++;
                  _ecore_con_cl_timer_update(cl);
                  e->client = cl;
                  e->data = inbuf;
                  e->size = inbuf_num;
                  ecore_event_add(ECORE_CON_EVENT_CLIENT_DATA, e,
                                  _ecore_con_event_client_data_free, NULL);
               }
          }

        if (lost_client && (!cl->delete_me))
          {
             /* we lost our client! */
             Ecore_Con_Event_Client_Del *e;

             e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
             if (e)
               {
                  cl->event_count++;
                  _ecore_con_cl_timer_update(cl);
                  e->client = cl;
                  ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                                  _ecore_con_event_client_del_free, NULL);
               }
          }

        if (lost_client)
          {
             cl->dead = EINA_TRUE;
             if (cl->fd_handler)
                ecore_main_fd_handler_del(cl->fd_handler);

             cl->fd_handler = NULL;
          }

        break;
     }
}

static Eina_Bool
_ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Client *cl;

   cl = data;
   if (cl->dead)
      return ECORE_CALLBACK_RENEW;

   if (cl->delete_me)
      return ECORE_CALLBACK_RENEW;

   if (cl->handshaking && ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ | ECORE_FD_WRITE))
     {
        if (!cl->ssl_state)
          {
             Ecore_Con_Event_Client_Add *add;

             add = calloc(1, sizeof(Ecore_Con_Event_Client_Add));
             if(add)
               {
/*cl->event_count++;*/
                  add->client = cl;
                  _ecore_con_cl_timer_update(cl);
                  ecore_event_add(ECORE_CON_EVENT_CLIENT_ADD, add,
                                  _ecore_con_event_client_add_free, NULL);
               }
          }
        else if (ecore_con_ssl_client_init(cl))
          {
             ERR("ssl handshaking failed!");
             /* we lost our client! */
             Ecore_Con_Event_Client_Del *e;

             cl->dead = EINA_TRUE;
             e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
             if (e)
               {
                  cl->event_count++;
                  _ecore_con_cl_timer_update(cl);
                  e->client = cl;
                  ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                                  _ecore_con_event_client_del_free, NULL);
               }

          }
     }

   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     _ecore_con_svr_cl_read(cl);

   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     _ecore_con_client_flush(cl);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_server_flush(Ecore_Con_Server *svr)
{
   int count, num;

   if (!svr->write_buf)
      return;

   /* check whether we need to write anything at all.
    * we must not write zero bytes with SSL_write() since it
    * causes undefined behaviour
    */
   if (svr->write_buf_size == svr->write_buf_offset)
      return;

   num = svr->write_buf_size - svr->write_buf_offset;

   if (svr->handshaking)
     {
        DBG("Continuing ssl handshake");
        if (ecore_con_ssl_server_init(svr))
          _ecore_con_server_kill(svr);
        return;
     }

   if (!(svr->type & ECORE_CON_SSL))
      count = write(svr->fd, svr->write_buf + svr->write_buf_offset, num);
   else
      count = ecore_con_ssl_server_write(svr, svr->write_buf + svr->write_buf_offset, num);

   if (count < 0)
     {
        /* we lost our server! */
        _ecore_con_server_kill(svr);
        return;
     }

   svr->write_buf_offset += count;
   if (svr->write_buf_offset >= svr->write_buf_size)
     {
        svr->write_buf_size = 0;
        svr->write_buf_offset = 0;
        free(svr->write_buf);
        svr->write_buf = NULL;
        if (svr->fd_handler)
           ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
     }
}

static void
_ecore_con_client_flush(Ecore_Con_Client *cl)
{
   int num, count = 0;

   if (!cl->buf)
      return;

   if (cl->handshaking)
     {
        if (ecore_con_ssl_client_init(cl))
          count = -1;

        _ecore_con_cl_timer_update(cl);
        return;
     }

   if (!count)
     {
        num = cl->buf_size - cl->buf_offset;
        if (!(cl->host_server->type & ECORE_CON_SSL))
           count = write(cl->fd, cl->buf + cl->buf_offset, num);
        else
           count = ecore_con_ssl_client_write(cl, cl->buf + cl->buf_offset, num);
     }

   if (count < 0)
     {
        if ((errno == EIO) || (errno == EBADF) || (errno == EPIPE) ||
            (errno == EINVAL) || (errno == ENOSPC) || (errno == ECONNREFUSED))
           if (!cl->delete_me)
             {
                /* we lost our client! */
                Ecore_Con_Event_Client_Del *e;

                e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
                if (e)
                  {
                     cl->event_count++;
                     _ecore_con_cl_timer_update(cl);
                     e->client = cl;
                     ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                                     _ecore_con_event_client_del_free, NULL);
                  }

                cl->dead = EINA_TRUE;
                if (cl->fd_handler)
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
        if (cl->fd_handler)
           ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
     }
}

static void
_ecore_con_event_client_add_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Add *e;

   e = ev;
   e->client->event_count--;
   if ((e->client->event_count == 0) &&
       (e->client->delete_me))
      ecore_con_client_del(e->client);

   free(e);
}

static void
_ecore_con_event_client_del_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Del *e;

   e = ev;
   e->client->event_count--;
   if ((e->client->event_count == 0) && (e->client->delete_me))
      ecore_con_client_del(e->client);

      free(e);
}

static void
_ecore_con_event_client_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   e->client->event_count--;
   if (e->data)
      free(e->data);

   if (((e->client->event_count == 0) && (e->client->delete_me)) ||
       ((e->client->host_server &&
         ((e->client->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
          (e->client->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
      ecore_con_client_del(e->client);

   free(e);
}

static void
_ecore_con_event_server_add_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Add *e;

   e = ev;
   e->server->event_count--;
   if ((e->server->event_count == 0) &&
       (e->server->delete_me))
      _ecore_con_server_free(e->server);

   free(e);
}

static void
_ecore_con_event_server_del_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Del *e;

   e = ev;
   e->server->event_count--;
   if ((e->server->event_count == 0) &&
       (e->server->delete_me))
      _ecore_con_server_free(e->server);

      free(e);
}

static void
_ecore_con_event_server_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   e->server->event_count--;
   if (e->data)
      free(e->data);

   if ((e->server->event_count == 0) &&
       (e->server->delete_me))
      _ecore_con_server_free(e->server);

   free(e);
}

static void
_ecore_con_lookup_done(void *data, Ecore_Con_Info *infos)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Lookup *lk;

   svr = data;
   lk = svr->data;

   if (infos)
      lk->done_cb(infos->info.ai_canonname, infos->ip,
                  infos->info.ai_addr, infos->info.ai_addrlen,
                  (void *)lk->data);
   else
      lk->done_cb(NULL, NULL, NULL, 0, (void *)lk->data);

   free(svr->name);
   free(lk);
   free(svr);
}
