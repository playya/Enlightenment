#include "e.h"

static EfsdConnection     *ec = NULL;
static Evas_List           fs_handlers = NULL;

static void _e_fs_fd_handle(int fd);

static void
_e_fs_fd_handle_a_la_cK(int fd)
{
   EfsdEvent ev;
   Evas_List l;
   
   while ((ec) && efsd_events_pending(ec))
     {		  
	ZERO(&ev, EfsdEvent, 1);
	
	if (efsd_next_event(ec, &ev) >= 0)
	  {
	     for (l = fs_handlers; l; l = l->next)
	       {
		  void (*func) (EfsdEvent *ev);
		  
		  func = l->data;
		  func(&ev);
	       }
	  }
	else
	  {
	     efsd_close(ec);
	     e_del_event_fd(fd);
	     ec = NULL;
	     /* FIXME: need to queue a popup dialog here saying */
	     /* efsd went wonky */
	     printf("EEEEEEEEEEK efsd went wonky\n");
	  }
	
	efsd_event_cleanup(&ev);
    }
}

static void
_e_fs_fd_handle(int fd)
{
   EfsdEvent ev;
   int i = 1;

   /* VERY nasty - sicne efas has no way of checkign If an event is in the */
   /* event queue waiting to be picked up - i cant loop and get the events */
   printf("_e_fs_fd_handle(%i)\n", fd);
   while (i >= 0)
     {
	fd_set    fdset;
	struct timeval tv;
	
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	select(fd + 1, &fdset, NULL, NULL, &tv);
	if (FD_ISSET(fd, &fdset))	  
	  {
	     i = efsd_next_event(ec, &ev);
	     if (i < 0)
	       {		  
		  efsd_close(ec);
		  e_del_event_fd(fd);
		  /* FIXME: need to queue a popup dialog here saying */
		  /* efsd went wonky */
		  printf("EEEEEEEEEEK efsd went wonky\n");
/*		  
		  ec = efsd_open();
		  if (ec)
		    e_add_event_fd(efsd_get_connection_fd(ec), 
				   _e_fs_fd_handle);
*/
	       }
	     if (i >= 0)
	       {
		  Evas_List l;
		  
		  for (l = fs_handlers; l; l = l->next)
		    {
		       void (*func) (EfsdEvent *ev);
		       
		       func = l->data;
		       func(&ev);
		    }
		  efsd_event_cleanup(&ev);
	       }
	  }
	else
	  i = -1;
     }
}

void
e_fs_add_event_handler(void (*func) (EfsdEvent *ev))
{
   if (!func) return;
   fs_handlers = evas_list_append(fs_handlers, func);
}

void
e_fs_init(void)
{
   int i;

   /* already have an efsd around? */
   ec = efsd_open();
   /* no - efsd around */
   if (!ec)
     {
	/* start efsd */
	e_exec_run("efsd");   
	for (i = 0; (!ec) && (i < 4); i++)
	  {
	    sleep(1);
	    ec = efsd_open();
	  }
     }
   /* after several atempts to talk to efsd - lets give up */
   if (!ec)
     {
	fprintf(stderr, "efsd is not running - please run efsd.\n");
	exit(-1);
     }
   e_add_event_fd(efsd_get_connection_fd(ec), _e_fs_fd_handle_a_la_cK);
}

EfsdConnection *
e_fs_get_connection(void)
{
   return ec;
}
