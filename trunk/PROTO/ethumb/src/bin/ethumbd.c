/**
 * @file
 *
 * Copyright (C) 2009 by ProFUSION embedded systems
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 * @author Rafael Antognolli <antognolli@profusion.mobi>
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include <Ethumb.h>
#include <Eina.h>
#include <Ecore_Getopt.h>
#include <Ecore.h>
#include <E_DBus.h>

#include "ethumbd_private.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_ID 2000000

#define DBG(...) EINA_ERROR_PDBG(__VA_ARGS__)
#define INF(...) EINA_ERROR_PINFO(__VA_ARGS__)
#define WRN(...) EINA_ERROR_PWARN(__VA_ARGS__)
#define ERR(...) EINA_ERROR_PERR(__VA_ARGS__)

static const char _ethumb_dbus_bus_name[] = "org.enlightenment.Ethumb";
static const char _ethumb_dbus_interface[] = "org.enlightenment.Ethumb";
static const char _ethumb_dbus_objects_interface[] = "org.enlightenment.Ethumb.objects";
static const char _ethumb_dbus_path[] = "/org/enlightenment/Ethumb";
static const char fdo_interface[] = "org.freedesktop.DBus";
static const char fdo_bus_name[] = "org.freedesktop.DBus";
static const char fdo_path[] = "/org/freedesktop/DBus";

struct _Ethumb_Setup
{
   struct
   {
      Eina_Bool fdo : 1;
      Eina_Bool size : 1;
      Eina_Bool format : 1;
      Eina_Bool aspect : 1;
      Eina_Bool crop : 1;
      Eina_Bool quality : 1;
      Eina_Bool compress : 1;
      Eina_Bool directory : 1;
      Eina_Bool category : 1;
      Eina_Bool frame : 1;
      Eina_Bool video_time : 1;
      Eina_Bool video_start : 1;
      Eina_Bool video_interval : 1;
      Eina_Bool video_ntimes : 1;
      Eina_Bool video_fps : 1;
      Eina_Bool document_page : 1;
   } flags;
   int fdo;
   int tw, th;
   int format;
   int aspect;
   float cx, cy;
   int quality;
   int compress;
   const char *directory;
   const char *category;
   const char *theme_file;
   const char *group;
   const char *swallow;
   float video_time;
   float video_start;
   float video_interval;
   int video_ntimes;
   int video_fps;
   int document_page;
};

struct _Ethumb_Request
{
   int id;
   const char *file, *key;
   const char *thumb, *thumb_key;
   struct _Ethumb_Setup setup;
};

struct _Ethumb_Object
{
   int used;
   const char *path;
   const char *client;
   Eina_List *queue;
   int nqueue;
   int id_count;
   int max_id;
   int min_id;
   E_DBus_Object *dbus_obj;
};

struct _Ethumb_Queue
{
   int count;
   int max_count;
   int nqueue;
   int last;
   int current;
   struct _Ethumb_Object *table;
   int *list;
};

struct _Ethumbd
{
   E_DBus_Connection *conn;
   E_DBus_Signal_Handler *name_owner_changed_handler;
   E_DBus_Interface *eiface, *objects_iface;
   E_DBus_Object *dbus_obj;
   Ecore_Idler *idler;
   struct _Ethumb_Request *processing;
   struct _Ethumb_Queue queue;
   int pipeout;
   int pipein;
   Ecore_Fd_Handler *fd_handler;
   double timeout;
   Ecore_Timer *timeout_timer;
};

struct _Ethumb_Object_Data
{
   int index;
   struct _Ethumbd *ed;
};

struct _Ethumb_DBus_Method_Table
{
   const char *name;
   const char *signature;
   const char *reply;
   E_DBus_Method_Cb function;
};

struct _Ethumb_DBus_Signal_Table
{
   const char *name;
   const char *signature;
};

const Ecore_Getopt optdesc = {
  "ethumbd",
  NULL,
  PACKAGE_VERSION,
  "(C) 2009 - ProFUSION embedded systems",
  "LGPL v3 - GNU Lesser General Public License",
  "Ethumb daemon.\n"
  "\n"
  "ethumbd uses the Ethumb library to create thumbnails for any "
  "program that requests it (now just by dbus).\n",
  0,
  {
    ECORE_GETOPT_STORE_DOUBLE
    ('t', "timeout", "finish ethumbd after <timeout> seconds of no activity."),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

static void _ethumb_dbus_generated_signal(struct _Ethumbd *ed, int *id, const char *thumb_path, const char *thumb_key, Eina_Bool success);

static int
_ethumbd_timeout_cb(void *data)
{
   struct _Ethumbd *ed = data;

   ecore_main_loop_quit();
   ed->timeout_timer = NULL;

   return 0;
}

static void
_ethumbd_timeout_start(struct _Ethumbd *ed)
{
   if (ed->timeout < 0)
     return;

   if (!ed->timeout_timer)
     ed->timeout_timer = ecore_timer_add(ed->timeout, _ethumbd_timeout_cb, ed);
}

static void
_ethumbd_timeout_stop(struct _Ethumbd *ed)
{
   if (!ed->timeout_timer)
     return;

   ecore_timer_del(ed->timeout_timer);
   ed->timeout_timer = NULL;
}

static int
_ethumb_dbus_check_id(struct _Ethumb_Object *eobject, int id)
{
   if (id < 0 || id > MAX_ID)
     return 0;

   if (eobject->min_id < eobject->max_id)
     return id < eobject->min_id || id > eobject->max_id;
   else if (eobject->min_id > eobject->max_id)
     return id < eobject->min_id && id > eobject->max_id;
   else
     return id != eobject->max_id;
}

static void
_ethumb_dbus_inc_max_id(struct _Ethumb_Object *eobject, int id)
{
   if (eobject->min_id < 0 && eobject->max_id < 0)
     eobject->min_id = id;

   eobject->max_id = id;
}

static void
_ethumb_dbus_inc_min_id(struct _Ethumb_Object *eobject)
{
   Eina_List *l;

   l = eobject->queue;
   while (l)
     {
	struct _Ethumb_Request *request = l->data;
	if (request->id >= 0)
	  {
	     eobject->min_id = request->id;
	     break;
	  }

	l = l->next;
     }

   if (!l)
     {
	eobject->min_id = -1;
	eobject->max_id = -1;
     }
}

int
_ethumbd_read_safe(int fd, void *buf, ssize_t size)
{
   ssize_t todo;
   char *p;

   todo = size;
   p = buf;

   while (todo > 0)
     {
	ssize_t r;

	r = read(fd, p, todo);
	if (r > 0)
	  {
	     todo -= r;
	     p += r;
	  }
	else if (r == 0)
	  return 0;
	else
	  {
	     if (errno == EINTR || errno == EAGAIN)
	       continue;
	     else
	       {
		  ERR("could not read from fd %d: %s",
		      fd, strerror(errno));
		  return 0;
	       }
	  }
     }

   return 1;
}

int
_ethumbd_write_safe(int fd, const void *buf, ssize_t size)
{
   ssize_t todo;
   const char *p;

   todo = size;
   p = buf;

   while (todo > 0)
     {
	ssize_t r;

	r = write(fd, p, todo);
	if (r > 0)
	  {
	     todo -= r;
	     p += r;
	  }
	else if (r == 0)
	  return 0;
	else
	  {
	     if (errno == EINTR || errno == EAGAIN)
	       continue;
	     else
	       {
		  ERR("could not write to fd %d: %s", fd, strerror(errno));
		  return 0;
	       }
	  }
     }

   return 1;
}

static void
_ethumbd_child_write_op_new(struct _Ethumbd *ed, int index)
{
   int id = ETHUMBD_OP_NEW;
   _ethumbd_write_safe(ed->pipeout, &id, sizeof(id));
   _ethumbd_write_safe(ed->pipeout, &index, sizeof(index));
}

static void
_ethumbd_child_write_op_del(struct _Ethumbd *ed, int index)
{
   int id = ETHUMBD_OP_DEL;
   _ethumbd_write_safe(ed->pipeout, &id, sizeof(id));
   _ethumbd_write_safe(ed->pipeout, &index, sizeof(index));
}

static void
_ethumbd_pipe_str_write(int fd, const char *str)
{
   int len;

   if (str)
     len = strlen(str) + 1;
   else
     len = 0;

   _ethumbd_write_safe(fd, &len, sizeof(len));
   _ethumbd_write_safe(fd, str, len);
}

static int
_ethumbd_pipe_str_read(int fd, char **str)
{
   int size;
   int r;
   char buf[PATH_MAX];

   r = _ethumbd_read_safe(fd, &size, sizeof(size));
   if (!r)
     {
	*str = NULL;
	return 0;
     }

   if (!size)
     {
	*str = NULL;
	return 1;
     }

   r = _ethumbd_read_safe(fd, buf, size);
   if (!r)
     {
	*str = NULL;
	return 0;
     }

   *str = strdup(buf);
   return 1;
}

static void
_ethumbd_child_write_op_generate(struct _Ethumbd *ed, int index, const char *path, const char *key, const char *thumb_path, const char *thumb_key)
{
   int id = ETHUMBD_OP_GENERATE;

   _ethumbd_write_safe(ed->pipeout, &id, sizeof(id));
   _ethumbd_write_safe(ed->pipeout, &index, sizeof(index));

   _ethumbd_pipe_str_write(ed->pipeout, path);
   _ethumbd_pipe_str_write(ed->pipeout, key);
   _ethumbd_pipe_str_write(ed->pipeout, thumb_path);
   _ethumbd_pipe_str_write(ed->pipeout, thumb_key);
}

static void
_generated_cb(struct _Ethumbd *ed, Eina_Bool success, const char *thumb_path, const char *thumb_key)
{
   int i = ed->queue.current;

   DBG("thumbnail ready at: \"%s:%s\"\n", thumb_path, thumb_key);

   if (ed->queue.table[i].used)
     _ethumb_dbus_generated_signal
       (ed, &ed->processing->id, thumb_path, thumb_key, success);
   eina_stringshare_del(ed->processing->file);
   eina_stringshare_del(ed->processing->key);
   eina_stringshare_del(ed->processing->thumb);
   eina_stringshare_del(ed->processing->thumb_key);
   free(ed->processing);
   ed->processing = NULL;
}

static int
_ethumbd_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   struct _Ethumbd *ed = data;
   Eina_Bool success;
   int r;
   char *thumb_path, *thumb_key;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR))
     {
	ERR("error on pipein! child exiting...\n");
	ed->fd_handler = NULL;
	ecore_main_loop_quit();
	return 0;
     }

   r = _ethumbd_read_safe(ed->pipein, &success, sizeof(success));
   if (!r)
     {
	ERR("ethumbd child exited!\n");
	ed->fd_handler = NULL;
	return 0;
     }

   r = _ethumbd_pipe_str_read(ed->pipein, &thumb_path);
   r = _ethumbd_pipe_str_read(ed->pipein, &thumb_key);
   _generated_cb(ed, success, thumb_path, thumb_key);

   free(thumb_path);
   free(thumb_key);

   return 1;
}

static void
_ethumbd_pipe_write_setup(int fd, int type, const void *data)
{
   const int *i_value;
   const float *f_value;

   _ethumbd_write_safe(fd, &type, sizeof(type));

   switch (type)
     {
      case ETHUMBD_FDO:
      case ETHUMBD_FORMAT:
      case ETHUMBD_ASPECT:
      case ETHUMBD_QUALITY:
      case ETHUMBD_COMPRESS:
      case ETHUMBD_SIZE_W:
      case ETHUMBD_SIZE_H:
      case ETHUMBD_DOCUMENT_PAGE:
      case ETHUMBD_VIDEO_NTIMES:
      case ETHUMBD_VIDEO_FPS:
	 i_value = data;
	 _ethumbd_write_safe(fd, i_value, sizeof(*i_value));
	 break;
      case ETHUMBD_CROP_X:
      case ETHUMBD_CROP_Y:
      case ETHUMBD_VIDEO_TIME:
      case ETHUMBD_VIDEO_START:
      case ETHUMBD_VIDEO_INTERVAL:
	 f_value = data;
	 _ethumbd_write_safe(fd, f_value, sizeof(*f_value));
	 break;
      case ETHUMBD_DIRECTORY:
      case ETHUMBD_CATEGORY:
      case ETHUMBD_FRAME_FILE:
      case ETHUMBD_FRAME_GROUP:
      case ETHUMBD_FRAME_SWALLOW:
	 _ethumbd_pipe_str_write(fd, data);
	 break;
      case ETHUMBD_SETUP_FINISHED:
	 break;
      default:
	 ERR("wrong ethumb setup parameter.\n");
     }
}

static void
_process_setup(struct _Ethumbd *ed)
{
   int op_id = ETHUMBD_OP_SETUP;
   int index = ed->queue.current;
   int fd = ed->pipeout;

   struct _Ethumb_Setup *setup = &ed->processing->setup;

   _ethumbd_write_safe(ed->pipeout, &op_id, sizeof(op_id));
   _ethumbd_write_safe(ed->pipeout, &index, sizeof(index));

   if (setup->flags.fdo)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_FDO, &setup->fdo);
   if (setup->flags.size)
     {
	_ethumbd_pipe_write_setup(fd, ETHUMBD_SIZE_W, &setup->tw);
	_ethumbd_pipe_write_setup(fd, ETHUMBD_SIZE_H, &setup->th);
     }
   if (setup->flags.format)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_FORMAT, &setup->format);
   if (setup->flags.aspect)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_ASPECT, &setup->aspect);
   if (setup->flags.crop)
     {
	_ethumbd_pipe_write_setup(fd, ETHUMBD_CROP_X, &setup->cx);
	_ethumbd_pipe_write_setup(fd, ETHUMBD_CROP_Y, &setup->cy);
     }
   if (setup->flags.quality)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_QUALITY, &setup->quality);
   if (setup->flags.compress)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_COMPRESS, &setup->compress);
   if (setup->flags.directory)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_DIRECTORY, setup->directory);
   if (setup->flags.category)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_CATEGORY, setup->category);
   if (setup->flags.frame)
     {
	_ethumbd_pipe_write_setup(fd, ETHUMBD_FRAME_FILE, setup->theme_file);
	_ethumbd_pipe_write_setup(fd, ETHUMBD_FRAME_GROUP, setup->group);
	_ethumbd_pipe_write_setup(fd, ETHUMBD_FRAME_SWALLOW, setup->swallow);
     }
   if (setup->flags.video_time)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_VIDEO_TIME, &setup->video_time);
   if (setup->flags.video_start)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_VIDEO_START, &setup->video_start);
   if (setup->flags.video_interval)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_VIDEO_INTERVAL,
			       &setup->video_interval);
   if (setup->flags.video_ntimes)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_VIDEO_NTIMES, &setup->video_ntimes);
   if (setup->flags.video_fps)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_VIDEO_FPS, &setup->video_fps);
   if (setup->flags.document_page)
     _ethumbd_pipe_write_setup(fd, ETHUMBD_DOCUMENT_PAGE,
			       &setup->document_page);
   _ethumbd_pipe_write_setup(fd, ETHUMBD_SETUP_FINISHED, NULL);


   if (setup->directory) eina_stringshare_del(setup->directory);
   if (setup->category) eina_stringshare_del(setup->category);
   if (setup->theme_file) eina_stringshare_del(setup->theme_file);
   if (setup->group) eina_stringshare_del(setup->group);
   if (setup->swallow) eina_stringshare_del(setup->swallow);

   free(ed->processing);
   ed->processing = NULL;
}

static void
_process_file(struct _Ethumbd *ed)
{
   _ethumbd_child_write_op_generate
     (ed, ed->queue.current, ed->processing->file,
      ed->processing->key, ed->processing->thumb, ed->processing->thumb_key);
}

static int
_get_next_on_queue(struct _Ethumb_Queue *queue)
{
   int i, index;
   struct _Ethumb_Object *eobject;

   i = queue->last;
   i++;
   if (i >= queue->count)
     i = 0;

   index = queue->list[i];
   eobject = &(queue->table[index]);
   while (!eobject->nqueue)
     {
	i = (i + 1) % queue->count;

	index = queue->list[i];
	eobject = &(queue->table[index]);
     }

   return queue->list[i];
}

static int
_process_queue_cb(void *data)
{
   struct _Ethumb_Object *eobject;
   int i;
   struct _Ethumbd *ed = data;
   struct _Ethumb_Queue *queue = &ed->queue;
   struct _Ethumb_Request *request;

   if (ed->processing)
     return 1;

   if (!queue->nqueue)
     {
	ed->idler = NULL;
	if (!queue->count)
	  _ethumbd_timeout_start(ed);
	ed->idler = NULL;
	return 0;
     }

   i = _get_next_on_queue(queue);
   eobject = &(queue->table[i]);

   request = eina_list_data_get(eobject->queue);
   eobject->queue = eina_list_remove_list(eobject->queue, eobject->queue);
   ed->queue.current = i;
   DBG("processing file: \"%s:%s\"...\n", request->file,
       request->key);
   ed->processing = request;

   if (request->id < 0)
     _process_setup(ed);
   else
     {
	_process_file(ed);
	_ethumb_dbus_inc_min_id(eobject);
     }
   eobject->nqueue--;
   queue->nqueue--;

   queue->last = i;

   return 1;
}

static void
_process_queue_start(struct _Ethumbd *ed)
{
   if (!ed->idler)
     ed->idler = ecore_idler_add(_process_queue_cb, ed);
}

static void
_process_queue_stop(struct _Ethumbd *ed)
{
   if (ed->idler)
     {
	ecore_idler_del(ed->idler);
	ed->idler = NULL;
     }
}

static int
_ethumb_table_append(struct _Ethumbd *ed)
{
   int i;
   char buf[1024];
   struct _Ethumb_Queue *q = &ed->queue;

   if (q->count == q->max_count)
     {
	int new_max = q->max_count + 5;
	int start, size;

	start = q->max_count;
	size = new_max - q->max_count;

	q->table = realloc(q->table, new_max * sizeof(struct _Ethumb_Object));
	q->list = realloc(q->list, new_max * sizeof(int));
	memset(&q->table[start], 0, size * sizeof(struct _Ethumb_Object));

	q->max_count = new_max;
     }

   for (i = 0; i < q->max_count; i++)
     {
	if (!q->table[i].used)
	  break;
     }

   snprintf(buf, sizeof(buf), "%s/%d", _ethumb_dbus_path, i);
   q->table[i].used = 1;
   q->table[i].path = eina_stringshare_add(buf);
   q->table[i].max_id = -1;
   q->table[i].min_id = -1;
   q->list[q->count] = i;
   q->count++;
   DBG("new object: %s, index = %d, count = %d\n", buf, i, q->count);

   return i;
}

static inline int
_get_index_for_path(const char *path)
{
   int i;
   int n;
   n = sscanf(path, "/org/enlightenment/Ethumb/%d", &i);
   if (!n)
     return -1;
   return i;
}

static void
_ethumb_table_del(struct _Ethumbd *ed, int i)
{
   int j;
   Eina_List *l;
   const Eina_List *il;
   struct _Ethumb_Queue *q = &ed->queue;
   struct _Ethumb_Object_Data *odata;

   eina_stringshare_del(q->table[i].path);

   l = q->table[i].queue;
   while (l)
     {
	struct _Ethumb_Request *request = l->data;
	eina_stringshare_del(request->file);
	eina_stringshare_del(request->key);
	eina_stringshare_del(request->thumb);
	eina_stringshare_del(request->thumb_key);
	free(request);
	l = eina_list_remove_list(l, l);
     }
   q->nqueue -= q->table[i].nqueue;

   il = e_dbus_object_interfaces_get(q->table[i].dbus_obj);
   while (il)
     {
	e_dbus_object_interface_detach(q->table[i].dbus_obj, il->data);
	il = e_dbus_object_interfaces_get(q->table[i].dbus_obj);
     }
   odata = e_dbus_object_data_get(q->table[i].dbus_obj);
   free(odata);
   e_dbus_object_free(q->table[i].dbus_obj);

   memset(&(q->table[i]), 0, sizeof(struct _Ethumb_Object));
   for (j = 0; j < q->count; j++)
     {
	if (q->list[j] == i)
	  q->list[j] = q->list[q->count - 1];
     }

   q->count--;
   _ethumbd_child_write_op_del(ed, i);
   if (!q->count && !ed->processing)
     _ethumbd_timeout_start(ed);
}

static void
_ethumb_table_clear(struct _Ethumbd *ed)
{
   int i;

   for (i = 0; i < ed->queue.max_count; i++)
     if (ed->queue.table[i].used)
       _ethumb_table_del(ed, i);
}

static void
_name_owner_changed_cb(void *data, DBusMessage *msg)
{
   DBusError err;
   struct _Ethumbd *ed = data;
   struct _Ethumb_Queue *q = &ed->queue;
   const char *name, *from, *to;
   int i;

   dbus_error_init(&err);
   if (!dbus_message_get_args(msg, &err,
			      DBUS_TYPE_STRING, &name,
			      DBUS_TYPE_STRING, &from,
			      DBUS_TYPE_STRING, &to,
			      DBUS_TYPE_INVALID))
     {
	ERR("could not get NameOwnerChanged arguments: %s: %s\n",
	    err.name, err.message);
	dbus_error_free(&err);
	return;
     }

   DBG("NameOwnerChanged: name = %s, from = %s, to = %s\n", name, from, to);

   if (from[0] == '\0' || to[0] != '\0')
     return;

   from = eina_stringshare_add(from);
   for (i = 0; i < q->max_count; i++)
     {
	if (q->table[i].used && q->table[i].client == from)
	  {
	     _ethumb_table_del(ed, i);
	     DBG("deleting [%d] from queue table.\n", i);
	  }
     }
}

static void
_ethumb_dbus_add_name_owner_changed_cb(struct _Ethumbd *ed)
{
   ed->name_owner_changed_handler = e_dbus_signal_handler_add
     (ed->conn, fdo_bus_name, fdo_path, fdo_interface, "NameOwnerChanged",
      _name_owner_changed_cb, ed);
}

DBusMessage *
_ethumb_dbus_ethumb_new_cb(E_DBus_Object *object, DBusMessage *msg)
{
   DBusMessage *reply;
   DBusMessageIter iter;
   E_DBus_Object *dbus_object;
   struct _Ethumb_Object_Data *odata;
   int i;
   const char *return_path = "";
   const char *client;
   struct _Ethumbd *ed;

   ed = e_dbus_object_data_get(object);
   client = dbus_message_get_sender(msg);
   if (!client)
     goto end_new;

   i = _ethumb_table_append(ed);

   odata = calloc(1, sizeof(*odata));
   odata->index = i;
   odata->ed = ed;

   ed->queue.table[i].client = eina_stringshare_add(client);
   return_path = ed->queue.table[i].path;

   dbus_object = e_dbus_object_add(ed->conn, return_path, odata);
   if (!dbus_object)
     {
	ERR("could not create dbus_object.\n");
	free(odata);
	return_path = "";
	goto end_new;
     }

   e_dbus_object_interface_attach(dbus_object, ed->objects_iface);
   ed->queue.table[i].dbus_obj = dbus_object;

   _ethumbd_child_write_op_new(ed, i);
   _ethumbd_timeout_stop(ed);

 end_new:
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(reply, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH,
				  &return_path);
   return reply;
}

static struct _Ethumb_DBus_Method_Table _ethumb_dbus_methods[] =
  {
    {"new", "", "o", _ethumb_dbus_ethumb_new_cb},
    {NULL, NULL, NULL, NULL}
  };

static const char *
_ethumb_dbus_get_bytearray(DBusMessageIter *iter)
{
   int el_type;
   int length;
   DBusMessageIter riter;
   const char *result;

   el_type = dbus_message_iter_get_element_type(iter);
   if (el_type != DBUS_TYPE_BYTE)
     {
	ERR("not an byte array element.\n");
	return NULL;
     }

   dbus_message_iter_recurse(iter, &riter);
   dbus_message_iter_get_fixed_array(&riter, &result, &length);

   if (result[0] == '\0')
     return NULL;
   else
     return eina_stringshare_add(result);
}

static void
_ethumb_dbus_append_bytearray(DBusMessageIter *iter, const char *string)
{
   DBusMessageIter viter;

   if (!string)
     string = "";

   dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &viter);
   dbus_message_iter_append_fixed_array
     (&viter, DBUS_TYPE_BYTE, &string, strlen(string) + 1);
   dbus_message_iter_close_container(iter, &viter);
}

DBusMessage *
_ethumb_dbus_queue_add_cb(E_DBus_Object *object, DBusMessage *msg)
{
   DBusMessage *reply;
   DBusMessageIter iter;
   const char *file, *key;
   const char *thumb, *thumb_key;
   struct _Ethumb_Object_Data *odata;
   struct _Ethumb_Object *eobject;
   struct _Ethumbd *ed;
   struct _Ethumb_Request *request;
   dbus_int32_t id = -1;

   dbus_message_iter_init(msg, &iter);
   dbus_message_iter_get_basic(&iter, &id);
   dbus_message_iter_next(&iter);
   file = _ethumb_dbus_get_bytearray(&iter);
   dbus_message_iter_next(&iter);
   key = _ethumb_dbus_get_bytearray(&iter);
   dbus_message_iter_next(&iter);
   thumb = _ethumb_dbus_get_bytearray(&iter);
   dbus_message_iter_next(&iter);
   thumb_key = _ethumb_dbus_get_bytearray(&iter);

   if (!file)
     {
	ERR("no filename given.\n");
	goto end;
     }

   odata = e_dbus_object_data_get(object);
   if (!odata)
     {
	ERR("could not get dbus_object data.\n");
	goto end;
     }

   ed = odata->ed;
   eobject = &(ed->queue.table[odata->index]);
   if (!_ethumb_dbus_check_id(eobject, id))
     goto end;
   request = calloc(1, sizeof(*request));
   request->id = id;
   request->file = file;
   request->key = key;
   request->thumb = thumb;
   request->thumb_key = thumb_key;
   eobject->queue = eina_list_append(eobject->queue, request);
   eobject->nqueue++;
   ed->queue.nqueue++;
   _ethumb_dbus_inc_max_id(eobject, id);

   _process_queue_start(ed);

 end:
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(reply, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &id);
   return reply;
}

DBusMessage *
_ethumb_dbus_queue_remove_cb(E_DBus_Object *object, DBusMessage *msg)
{
   DBusMessage *reply;
   DBusMessageIter iter;
   dbus_int32_t id;
   struct _Ethumb_Object_Data *odata;
   struct _Ethumb_Object *eobject;
   struct _Ethumb_Request *request;
   struct _Ethumbd *ed;
   dbus_bool_t r = 0;
   Eina_List *l;

   dbus_message_iter_init(msg, &iter);
   dbus_message_iter_get_basic(&iter, &id);

   odata = e_dbus_object_data_get(object);
   if (!odata)
     {
	ERR("could not get dbus_object data.\n");
	goto end;
     }

   ed = odata->ed;
   eobject = &ed->queue.table[odata->index];
   l = eobject->queue;
   while (l)
     {
	request = l->data;
	if (id == request->id)
	  break;
	l = l->next;
     }

   if (l)
     {
	r = 1;
	eina_stringshare_del(request->file);
	eina_stringshare_del(request->key);
	eina_stringshare_del(request->thumb);
	eina_stringshare_del(request->thumb_key);
	free(request);
	eobject->queue = eina_list_remove_list(eobject->queue, l);
	eobject->nqueue--;
	ed->queue.nqueue--;
	_ethumb_dbus_inc_min_id(eobject);
     }

 end:
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(reply, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &r);
   return reply;
}

DBusMessage *
_ethumb_dbus_queue_clear_cb(E_DBus_Object *object, DBusMessage *msg)
{
   DBusMessage *reply;
   struct _Ethumb_Object_Data *odata;
   struct _Ethumb_Object *eobject;
   struct _Ethumbd *ed;
   Eina_List *l;

   odata = e_dbus_object_data_get(object);
   if (!odata)
     {
	ERR("could not get dbus_object data.\n");
	goto end;
     }

   ed = odata->ed;
   eobject = &ed->queue.table[odata->index];
   l = eobject->queue;
   while (l)
     {
	struct _Ethumb_Request *request = l->data;
	eina_stringshare_del(request->file);
	eina_stringshare_del(request->key);
	eina_stringshare_del(request->thumb);
	eina_stringshare_del(request->thumb_key);
	free(request);
	l = eina_list_remove_list(l, l);
     }
   ed->queue.nqueue -= eobject->nqueue;
   eobject->nqueue = 0;

 end:
   reply = dbus_message_new_method_return(msg);
   return reply;
}

DBusMessage *
_ethumb_dbus_delete_cb(E_DBus_Object *object, DBusMessage *msg)
{
   DBusMessage *reply;
   DBusMessageIter iter;
   struct _Ethumb_Object_Data *odata;
   struct _Ethumbd *ed;

   dbus_message_iter_init(msg, &iter);
   reply = dbus_message_new_method_return(msg);

   odata = e_dbus_object_data_get(object);
   if (!odata)
     {
	ERR("could not get dbus_object data for del_cb.\n");
	return reply;
     }
   ed = odata->ed;
   _ethumb_table_del(ed, odata->index);
   free(odata);

   return reply;
}

static int
_ethumb_dbus_fdo_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request __UNUSED__)
{
   int type;
   dbus_int32_t fdo;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for fdo_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &fdo);
   DBG("setting fdo to: %d\n", fdo);

   return 1;
}

static int
_ethumb_dbus_size_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   DBusMessageIter oiter;
   int type;
   dbus_int32_t w, h;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_STRUCT)
     {
	ERR("invalid param for size_set.\n");
	return 0;
     }

   dbus_message_iter_recurse(iter, &oiter);
   dbus_message_iter_get_basic(&oiter, &w);
   dbus_message_iter_next(&oiter);
   dbus_message_iter_get_basic(&oiter, &h);
   DBG("setting size to: %dx%d\n", w, h);
   request->setup.flags.size = 1;
   request->setup.tw = w;
   request->setup.th = h;

   return 1;
}

static int
_ethumb_dbus_format_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   dbus_int32_t format;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for format_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &format);
   DBG("setting format to: %d\n", format);
   request->setup.flags.format = 1;
   request->setup.format = format;

   return 1;
}

static int
_ethumb_dbus_aspect_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   dbus_int32_t aspect;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for aspect_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &aspect);
   DBG("setting aspect to: %d\n", aspect);
   request->setup.flags.aspect = 1;
   request->setup.aspect = aspect;

   return 1;
}

static int
_ethumb_dbus_crop_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   DBusMessageIter oiter;
   int type;
   double x, y;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_STRUCT)
     {
	ERR("invalid param for crop_set.\n");
	return 0;
     }

   dbus_message_iter_recurse(iter, &oiter);
   dbus_message_iter_get_basic(&oiter, &x);
   dbus_message_iter_next(&oiter);
   dbus_message_iter_get_basic(&oiter, &y);
   DBG("setting crop to: %3.2f,%3.2f\n", x, y);
   request->setup.flags.crop = 1;
   request->setup.cx = x;
   request->setup.cy = y;

   return 1;
}

static int
_ethumb_dbus_quality_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   dbus_int32_t quality;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for quality_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &quality);
   DBG("setting quality to: %d\n", quality);
   request->setup.flags.quality = 1;
   request->setup.quality = quality;

   return 1;
}


static int
_ethumb_dbus_compress_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   dbus_int32_t compress;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for compress_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &compress);
   DBG("setting compress to: %d\n", compress);
   request->setup.flags.compress = 1;
   request->setup.compress = compress;

   return 1;
}

static int
_ethumb_dbus_frame_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   DBusMessageIter oiter;
   int type;
   const char *file, *group, *swallow;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_STRUCT)
     {
	ERR("invalid param for frame_set.\n");
	return 0;
     }

   dbus_message_iter_recurse(iter, &oiter);
   file = _ethumb_dbus_get_bytearray(&oiter);
   dbus_message_iter_next(&oiter);
   group = _ethumb_dbus_get_bytearray(&oiter);
   dbus_message_iter_next(&oiter);
   swallow = _ethumb_dbus_get_bytearray(&oiter);
   DBG("setting frame to \"%s:%s:%s\"\n", file, group, swallow);
   request->setup.flags.frame = 1;
   request->setup.theme_file = eina_stringshare_add(file);
   request->setup.group = eina_stringshare_add(group);
   request->setup.swallow = eina_stringshare_add(swallow);

   return 1;
}

static int
_ethumb_dbus_directory_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   const char *directory;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_ARRAY)
     {
	ERR("invalid param for dir_path_set.\n");
	return 0;
     }

   directory = _ethumb_dbus_get_bytearray(iter);
   DBG("setting directory to: %s\n", directory);
   request->setup.flags.directory = 1;
   request->setup.directory = eina_stringshare_add(directory);

   return 1;
}

static int
_ethumb_dbus_category_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   const char *category;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_ARRAY)
     {
	ERR("invalid param for category.\n");
	return 0;
     }

   category = _ethumb_dbus_get_bytearray(iter);
   DBG("setting category to: %s\n", category);
   request->setup.flags.category = 1;
   request->setup.category = eina_stringshare_add(category);

   return 1;
}

static int
_ethumb_dbus_video_time_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   double video_time;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_DOUBLE)
     {
	ERR("invalid param for video_time_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &video_time);
   DBG("setting video_time to: %3.2f\n", video_time);
   request->setup.flags.video_time = 1;
   request->setup.video_time = video_time;

   return 1;
}

static int
_ethumb_dbus_video_start_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   double video_start;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_DOUBLE)
     {
	ERR("invalid param for video_start_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &video_start);
   DBG("setting video_start to: %3.2f\n", video_start);
   request->setup.flags.video_start = 1;
   request->setup.video_start = video_start;

   return 1;
}

static int
_ethumb_dbus_video_interval_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   double video_interval;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_DOUBLE)
     {
	ERR("invalid param for video_interval_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &video_interval);
   DBG("setting video_interval to: %3.2f\n", video_interval);
   request->setup.flags.video_interval = 1;
   request->setup.video_interval = video_interval;

   return 1;
}

static int
_ethumb_dbus_video_ntimes_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   int video_ntimes;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for video_ntimes_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &video_ntimes);
   DBG("setting video_ntimes to: %3.2d\n", video_ntimes);
   request->setup.flags.video_ntimes = 1;
   request->setup.video_ntimes = video_ntimes;

   return 1;
}

static int
_ethumb_dbus_video_fps_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   int video_fps;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for video_fps_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &video_fps);
   DBG("setting video_fps to: %3.2d\n", video_fps);
   request->setup.flags.video_fps = 1;
   request->setup.video_fps = video_fps;

   return 1;
}

static int
_ethumb_dbus_document_page_set(struct _Ethumb_Object *eobject __UNUSED__, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   int type;
   dbus_int32_t document_page;

   type = dbus_message_iter_get_arg_type(iter);
   if (type != DBUS_TYPE_INT32)
     {
	ERR("invalid param for document_page_set.\n");
	return 0;
     }

   dbus_message_iter_get_basic(iter, &document_page);
   DBG("setting document_page to: %d\n", document_page);
   request->setup.flags.document_page = 1;
   request->setup.document_page = document_page;

   return 1;
}

static struct
{
   const char *option;
   int (*setup_func)(struct _Ethumb_Object *eobject, DBusMessageIter *iter, struct _Ethumb_Request *request);
} _option_cbs[] = {
  {"fdo", _ethumb_dbus_fdo_set},
  {"size", _ethumb_dbus_size_set},
  {"format", _ethumb_dbus_format_set},
  {"aspect", _ethumb_dbus_aspect_set},
  {"crop", _ethumb_dbus_crop_set},
  {"quality", _ethumb_dbus_quality_set},
  {"compress", _ethumb_dbus_compress_set},
  {"frame", _ethumb_dbus_frame_set},
  {"directory", _ethumb_dbus_directory_set},
  {"category", _ethumb_dbus_category_set},
  {"video_time", _ethumb_dbus_video_time_set},
  {"video_start", _ethumb_dbus_video_start_set},
  {"video_interval", _ethumb_dbus_video_interval_set},
  {"video_ntimes", _ethumb_dbus_video_ntimes_set},
  {"video_fps", _ethumb_dbus_video_fps_set},
  {"document_page", _ethumb_dbus_document_page_set},
  {NULL, NULL}
};

static int
_ethumb_dbus_ethumb_setup_parse_element(struct _Ethumb_Object *eobject, DBusMessageIter *iter, struct _Ethumb_Request *request)
{
   DBusMessageIter viter, diter;
   const char *option;
   int i, r;

   dbus_message_iter_recurse(iter, &diter);
   dbus_message_iter_get_basic(&diter, &option);
   dbus_message_iter_next(&diter);

   r = 0;
   for (i = 0; _option_cbs[i].option; i++)
     if (!strcmp(_option_cbs[i].option, option))
       {
	  r = 1;
	  break;
       }

   if (!r)
     {
	ERR("ethumb_setup invalid option: %s\n", option);
	return 0;
     }

   dbus_message_iter_recurse(&diter, &viter);
   return _option_cbs[i].setup_func(eobject, &viter, request);
}

DBusMessage *
_ethumb_dbus_ethumb_setup_cb(E_DBus_Object *object, DBusMessage *msg)
{
   DBusMessage *reply;
   DBusMessageIter iter, aiter;
   struct _Ethumb_Object_Data *odata;
   struct _Ethumbd *ed;
   struct _Ethumb_Object *eobject;
   struct _Ethumb_Request *request;
   dbus_bool_t r = 0;
   int atype;

   dbus_message_iter_init(msg, &iter);
   if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
     {
	ERR("wrong parameters.\n");
	goto end;
     }

   odata = e_dbus_object_data_get(object);
   if (!odata)
     {
	ERR("could not get dbus_object data for setup_cb.\n");
	goto end;
     }

   ed = odata->ed;
   eobject = &ed->queue.table[odata->index];

   request = calloc(1, sizeof(*request));
   request->id = -1;
   dbus_message_iter_recurse(&iter, &aiter);
   atype = dbus_message_iter_get_arg_type(&aiter);

   r = 1;
   while (atype != DBUS_TYPE_INVALID)
     {
	if (!_ethumb_dbus_ethumb_setup_parse_element(eobject, &aiter, request))
	  r = 0;
	dbus_message_iter_next(&aiter);
	atype = dbus_message_iter_get_arg_type(&aiter);
     }

   eobject->queue = eina_list_append(eobject->queue, request);
   eobject->nqueue++;
   ed->queue.nqueue++;

 end:
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(reply, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &r);

   return reply;
}

static void
_ethumb_dbus_generated_signal(struct _Ethumbd *ed, int *id, const char *thumb_path, const char *thumb_key, Eina_Bool success)
{
   DBusMessage *signal;
   int current;
   const char *opath;
   DBusMessageIter iter;
   dbus_bool_t value;
   dbus_int32_t id32;

   value = success;
   id32 = *id;

   current = ed->queue.current;
   opath = ed->queue.table[current].path;
   signal = dbus_message_new_signal
     (opath, _ethumb_dbus_objects_interface, "generated");

   dbus_message_iter_init_append(signal, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &id32);
   _ethumb_dbus_append_bytearray(&iter, thumb_path);
   _ethumb_dbus_append_bytearray(&iter, thumb_key);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &value);

   e_dbus_message_send(ed->conn, signal, NULL, -1, NULL);
   dbus_message_unref(signal);
}

static struct _Ethumb_DBus_Method_Table _ethumb_dbus_objects_methods[] = {
  {"queue_add", "iayayayay", "i", _ethumb_dbus_queue_add_cb},
  {"queue_remove", "i", "b", _ethumb_dbus_queue_remove_cb},
  {"clear_queue", "", "", _ethumb_dbus_queue_clear_cb},
  {"ethumb_setup", "a{sv}", "b", _ethumb_dbus_ethumb_setup_cb},
  {"delete", "", "", _ethumb_dbus_delete_cb},
  {NULL, NULL, NULL, NULL}
};

static struct _Ethumb_DBus_Signal_Table _ethumb_dbus_objects_signals[] = {
  {"generated", "xayayb"},
  {NULL, NULL}
};

static int
_ethumb_dbus_interface_elements_add(E_DBus_Interface *iface, struct _Ethumb_DBus_Method_Table *mtable, struct _Ethumb_DBus_Signal_Table *stable)
{
   int i = -1;
   while (mtable && mtable[++i].name != NULL)
     if (!e_dbus_interface_method_add(iface,
				      mtable[i].name,
				      mtable[i].signature,
				      mtable[i].reply,
				      mtable[i].function))
       return 0;

   i = -1;
   while (stable && stable[++i].name != NULL)
     if (!e_dbus_interface_signal_add(iface,
				      stable[i].name,
				      stable[i].signature))
       return 0;
   return 1;
}

static void
_ethumb_dbus_request_name_cb(void *data, DBusMessage *msg __UNUSED__, DBusError *err)
{
   E_DBus_Object *dbus_object;
   struct _Ethumbd *ed = data;
   int r;

   if (dbus_error_is_set(err))
     {
	ERR("request name error: %s\n", err->message);
	dbus_error_free(err);
	e_dbus_connection_close(ed->conn);
	return;
     }

   dbus_object = e_dbus_object_add(ed->conn, _ethumb_dbus_path, ed);
   if (!dbus_object)
     return;
   ed->dbus_obj = dbus_object;
   ed->eiface = e_dbus_interface_new(_ethumb_dbus_interface);
   if (!ed->eiface)
     {
	ERR("could not create interface.\n");
	return;
     }
   r = _ethumb_dbus_interface_elements_add(ed->eiface,
					   _ethumb_dbus_methods, NULL);
   if (!r)
     {
	ERR("could not add methods to the interface.\n");
	e_dbus_interface_unref(ed->eiface);
	return;
     }
   e_dbus_object_interface_attach(dbus_object, ed->eiface);

   ed->objects_iface = e_dbus_interface_new(_ethumb_dbus_objects_interface);
   if (!ed->objects_iface)
     {
	ERR("could not create interface.\n");
	return;
     }

   r = _ethumb_dbus_interface_elements_add(ed->objects_iface,
					   _ethumb_dbus_objects_methods,
					   _ethumb_dbus_objects_signals);
   if (!r)
     {
	ERR("ERROR: could not setup objects interface methods.\n");
	e_dbus_interface_unref(ed->objects_iface);
	return;
     }

   _ethumb_dbus_add_name_owner_changed_cb(ed);

   _ethumbd_timeout_start(ed);
}

static int
_ethumb_dbus_setup(struct _Ethumbd *ed)
{
   e_dbus_request_name
     (ed->conn, _ethumb_dbus_bus_name, 0, _ethumb_dbus_request_name_cb, ed);

   return 1;
}

static void
_ethumb_dbus_finish(struct _Ethumbd *ed)
{
   _process_queue_stop(ed);
   _ethumb_table_clear(ed);
   e_dbus_signal_handler_del(ed->conn, ed->name_owner_changed_handler);
   e_dbus_interface_unref(ed->objects_iface);
   e_dbus_interface_unref(ed->eiface);
   e_dbus_object_free(ed->dbus_obj);
   free(ed->queue.table);
   free(ed->queue.list);
}

static int
_ethumbd_spawn(struct _Ethumbd *ed)
{
   int pparent[2]; // parent writes here
   int pchild[2]; // child writes here
   int pid;

   if (pipe(pparent) == -1)
     {
	ERR("could not create parent pipe.\n");
	return 0;
     }

   if (pipe(pchild) == -1)
     {
	ERR("could not create child pipe.\n");
	return 0;
     }

   pid = fork();
   if (pid == -1)
     {
	ERR("fork error.\n");
	return 0;
     }

   if (pid == 0)
     {
	close(pparent[1]);
	close(pchild[0]);
	ethumbd_child_start(pparent[0], pchild[1]);
	return 2;
     }
   else
     {
	close(pparent[0]);
	close(pchild[1]);
	ed->pipeout = pparent[1];
	ed->pipein = pchild[0];
	ed->fd_handler = ecore_main_fd_handler_add
	  (ed->pipein, ECORE_FD_READ | ECORE_FD_ERROR,
	   _ethumbd_fd_handler, ed, NULL, NULL);
	return 1;
     }
}

int
main(int argc, char *argv[])
{
   Eina_Bool quit_option = 0;
   int exit_value = 0;
   int arg_index;
   struct _Ethumbd ed;
   int child;
   double timeout = -1;

   memset(&ed, 0, sizeof(ed));
   ecore_init();
   eina_init();

   ethumb_init();

   child = _ethumbd_spawn(&ed);
   if (!child)
     {
	exit_value = -6;
	goto finish;
     }

   if (child == 2)
     {
	exit_value = 0;
	goto finish;
     }

   if (!e_dbus_init())
     {
	ERR("could not init e_dbus.\n");
	exit_value = -1;
	goto finish;
     }

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_DOUBLE(timeout),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (arg_index < 0)
     {
	ERR("Could not parse arguments.\n");
	exit_value = -2;
	goto finish;
     }

   if (quit_option)
     goto finish;

   ed.conn = e_dbus_bus_get(DBUS_BUS_SESSION);
   if (!ed.conn)
     {
	ERR("could not connect to session bus.\n");
	exit_value = -3;
	goto finish_edbus;
     }

   ed.timeout = timeout;

   if (!_ethumb_dbus_setup(&ed))
     {
	e_dbus_connection_close(ed.conn);
	ERR("could not setup dbus connection.\n");
	exit_value = -5;
	goto finish_edbus;
     }

   ecore_main_loop_begin();
   _ethumb_dbus_finish(&ed);

 finish_edbus:
   e_dbus_shutdown();
 finish:
   ethumb_shutdown();
   eina_init();
   ecore_shutdown();
   return exit_value;
}
