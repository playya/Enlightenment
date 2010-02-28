/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * Simple dns lookup
 *
 * http://www.faqs.org/rfcs/rfc1035.html
 * man resolv.conf
 *
 * And a sneakpeek at ares, ftp://athena-dist.mit.edu/pub/ATHENA/ares/
 */
/*
 * TODO
 * * Check env LOCALDOMAIN to override search
 * * Check env RES_OPTIONS to override options
 *
 * * Read /etc/host.conf
 * * host.conf env
 *   RESOLV_HOST_CONF, RESOLV_SERV_ORDER
 * * Check /etc/hosts
 *
 * * Caching
 *   We should store all names returned when CNAME, might have different ttl.
 *   Check against search and hostname when querying cache?
 * * Remember all querys and delete them on shutdown
 *
 * * Need more buffer overflow checks.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#ifdef __OpenBSD__
# include <sys/types.h>
#endif

#ifndef _WIN32
# include <sys/socket.h>
# include <arpa/inet.h>
# include <arpa/nameser.h>
#include <netdb.h>
#else
# include <ws2tcpip.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_con_private.h"

typedef struct _CB_Data CB_Data;

struct _CB_Data
{
   EINA_INLIST;
   void (*cb_done) (void *data, struct hostent *hostent);
   void *data;
   Ecore_Fd_Handler *fdh;
   pid_t pid;
   Ecore_Event_Handler *handler;
   int fd2;
};

static void _ecore_con_dns_readdata(CB_Data *cbdata);
static void _ecore_con_dns_slave_free(CB_Data *cbdata);
static int _ecore_con_dns_data_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_dns_exit_handler(void *data, int type __UNUSED__, void *event);
static Eina_Bool _ecore_con_write_safe(int fd, void *data, size_t length);

static int dns_init = 0;
static CB_Data *dns_slaves = NULL;

int
ecore_con_dns_init(void)
{
   dns_init++;
   return dns_init;
}

int
ecore_con_dns_shutdown(void)
{
   dns_init--;
   if (dns_init == 0)
     {
	while (dns_slaves) _ecore_con_dns_slave_free(dns_slaves);
     }
   return dns_init;
}

int
ecore_con_dns_lookup(const char *name,
		     void (*done_cb) (void *data, struct hostent *hostent),
		     void *data)
{
   CB_Data *cbdata;
   int fd[2];

   if (pipe(fd) < 0) return 0;
   cbdata = calloc(1, sizeof(CB_Data));
   if (!cbdata)
     {
	close(fd[0]);
	close(fd[1]);
	return 0;
     }
   cbdata->cb_done = done_cb;
   cbdata->data = data;
   cbdata->fd2 = fd[1];
   if (!(cbdata->fdh = ecore_main_fd_handler_add(fd[0], ECORE_FD_READ,
						 _ecore_con_dns_data_handler,
						 cbdata,
						 NULL, NULL)))
     {
	free(cbdata);
	close(fd[0]);
	close(fd[1]);
	return 0;
     }

   if ((cbdata->pid = fork()) == 0)
     {
	struct hostent *he;

	/* CHILD */
	he = gethostbyname(name);
	if (he)
	  {
	     struct in_addr addr;

	     memcpy((struct in_addr *)&addr, he->h_addr,
		    sizeof(struct in_addr));
	     _ecore_con_write_safe(fd[1], &(addr.s_addr), sizeof(in_addr_t));
	  }
	else
	  {
	     _ecore_con_write_safe(fd[1], "", 1);
	  }
	close(fd[1]);
# ifdef __USE_ISOC99
	_Exit(0);
# else
	_exit(0);
# endif
     }
   /* PARENT */
   cbdata->handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _ecore_con_dns_exit_handler, cbdata);
   close(fd[1]);
   if (!cbdata->handler)
     {
	ecore_main_fd_handler_del(cbdata->fdh);
	free(cbdata);
	close(fd[0]);
	return 0;
     }
   dns_slaves = (CB_Data *) eina_inlist_append(EINA_INLIST_GET(dns_slaves), EINA_INLIST_GET(cbdata));
   return 1;
}

static void
_ecore_con_dns_readdata(CB_Data *cbdata)
{
   struct hostent he;
   struct in_addr addr;
   char *addr2;
   ssize_t size;

   size = read(ecore_main_fd_handler_fd_get(cbdata->fdh), &(addr.s_addr),
	       sizeof(in_addr_t));
   if (size == sizeof(in_addr_t))
     {
	addr2 = (char *)&addr;
	he.h_addrtype = AF_INET;
	he.h_length = sizeof(in_addr_t);
	he.h_addr_list = &addr2;
	cbdata->cb_done(cbdata->data, &he);
     }
   else
     cbdata->cb_done(cbdata->data, NULL);
   cbdata->cb_done = NULL;
}

static void
_ecore_con_dns_slave_free(CB_Data *cbdata)
{
   dns_slaves = (CB_Data *) eina_inlist_remove(EINA_INLIST_GET(dns_slaves), EINA_INLIST_GET(cbdata));
   close(ecore_main_fd_handler_fd_get(cbdata->fdh));
   ecore_main_fd_handler_del(cbdata->fdh);
   ecore_event_handler_del(cbdata->handler);
   free(cbdata);
}

static int
_ecore_con_dns_data_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   CB_Data *cbdata;

   cbdata = data;
   if (cbdata->cb_done)
     {
	if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
	  _ecore_con_dns_readdata(cbdata);
	else
	  {
	     cbdata->cb_done(cbdata->data, NULL);
	     cbdata->cb_done = NULL;
	  }
     }
   _ecore_con_dns_slave_free(cbdata);
   return 0;
}

static int
_ecore_con_dns_exit_handler(void *data, int type __UNUSED__, void *event)
{
   CB_Data *cbdata;
   Ecore_Exe_Event_Del *ev;

   ev = event;
   cbdata = data;
   if (cbdata->pid != ev->pid) return 1;
   return 0;
   _ecore_con_dns_slave_free(cbdata);
   return 0;
}

static Eina_Bool
_ecore_con_write_safe(int fd, void *data, size_t length)
{
   char *buf = data;
   ssize_t todo = (ssize_t)length;

   while (todo > 0)
     {
	ssize_t r = write(fd, buf, todo);
	if (r > 0)
	  {
	     todo -= r;
	     buf += r;
	  }
	else if ((r < 0) && (errno == EINTR))
	  continue;
	else
	  {
	     ERR("could not write to fd %d: %s", fd, strerror(errno));
	     return EINA_FALSE;
	  }
     }
   return EINA_TRUE;
}
