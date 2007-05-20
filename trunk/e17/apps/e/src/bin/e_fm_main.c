/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <utime.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <glob.h>
#include <errno.h>
#include <signal.h>
#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <Ecore_File.h>
#include <Evas.h>

/* FIXME: things to add to the slave enlightenment_fm process and ipc to e:
 * 
 * * reporting results of fop's (current status - what has been don, what failed etc.)
 * * dbus removable device monitoring (in e17 itself now via ecore_dbus - move to enlightenment_fm and ipc removable device add/del and anything else)
 * * mount/umount of removable devices (to go along with removable device support - put it in here and message back mount success/failure and where it is now mounted - remove from e17 itself)
 * 
 */

#define DEF_SYNC_NUM 8
#define DEF_ROUND_TRIP 0.05
#define DEF_ROUND_TRIP_TOLERANCE 0.01
#define DEF_MOD_BACKOFF 0.2

typedef struct _E_Dir E_Dir;
typedef struct _E_Fop E_Fop;
typedef struct _E_Mod E_Mod;

struct _E_Dir
{
   int                 id;
   const char         *dir;
   Ecore_File_Monitor *mon;
   int                 mon_ref;
   E_Dir              *mon_real;
   Evas_List          *fq;
   Ecore_Idler        *idler;
   int                 dot_order;
   int                 sync;
   double              sync_time;
   int                 sync_num;
   Evas_List          *recent_mods;
   Ecore_Timer        *recent_clean;
   unsigned char       cleaning : 1;
};

struct _E_Fop
{
   int                 id;
   const char         *src;
   const char         *dst;
   const char         *rel;
   int                 rel_to;
   int                 x, y;
   unsigned char       del_after : 1;
   unsigned char       gone_bad : 1;
   Ecore_Idler        *idler;
   void               *data;
};

struct _E_Mod
{
   const char    *path;
   double         timestamp;
   unsigned char  add : 1;
   unsigned char  del : 1;
   unsigned char  mod : 1;
   unsigned char  done : 1;
};

/* local subsystem functions */
static int _e_ipc_init(void);
static int _e_ipc_cb_server_add(void *data, int type, void *event);
static int _e_ipc_cb_server_del(void *data, int type, void *event);
static int _e_ipc_cb_server_data(void *data, int type, void *event);

static void _e_cb_file_monitor(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);
static int _e_cb_recent_clean(void *data);

static void _e_file_add_mod(E_Dir *ed, const char *path, int op, int listing);
static void _e_file_add(E_Dir *ed, const char *path, int listing);
static void _e_file_del(E_Dir *ed, const char *path);
static void _e_file_mod(E_Dir *ed, const char *path);
static void _e_file_mon_dir_del(E_Dir *ed, const char *path);
static void _e_file_mon_list_sync(E_Dir *ed);

static int _e_cb_file_mon_list_idler(void *data);
static int _e_cb_fop_rm_idler(void *data);
static int _e_cb_fop_trash_idler(void *data);
static int _e_cb_fop_mv_idler(void *data);
static int _e_cb_fop_cp_idler(void *data);
static char *_e_str_list_remove(Evas_List **list, char *str);
static void _e_path_fix_order(const char *path, const char *rel, int rel_to, int x, int y);
static void _e_dir_del(E_Dir *ed);

/* local subsystem globals */
static Ecore_Ipc_Server *_e_ipc_server = NULL;

static Evas_List *_e_dirs = NULL;
static Evas_List *_e_fops = NULL;
static int _e_sync_num = 0;

/* externally accessible functions */
int
main(int argc, char **argv)
{
   int i;

   for (i = 1; i < argc; i++)
     {
	if ((!strcmp(argv[i], "-h")) ||
	    (!strcmp(argv[i], "-help")) ||
	    (!strcmp(argv[i], "--help")))
	  {
	     printf(
		    "This is an internal tool for Enlightenment.\n"
		    "do not use it.\n"
		    );
	     exit(0);
	  }
     }

   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);
   ecore_file_init();
   ecore_ipc_init();

   if (_e_ipc_init()) ecore_main_loop_begin();
   
   if (_e_ipc_server)
     {
	ecore_ipc_server_del(_e_ipc_server);
	_e_ipc_server = NULL;
     }

   ecore_ipc_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   
   return 0;
}

/* local subsystem functions */
static int
_e_ipc_init(void)
{
   char *sdir;
   
   sdir = getenv("E_IPC_SOCKET");
   if (!sdir)
     {
	printf("The E_IPC_SOCKET environment variable is not set. This is\n"
	       "exported by Enlightenment to all processes it launches.\n"
	       "This environment variable must be set and must point to\n"
	       "Enlightenment's IPC socket file (minus port number).\n");
	return 0;
     }
   _e_ipc_server = ecore_ipc_server_connect(ECORE_IPC_LOCAL_SYSTEM, sdir, 0, NULL);
   if (!_e_ipc_server)
     {
	printf("Cannot connect to enlightenment - abort\n");
	return 0;
     }
   
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, _e_ipc_cb_server_add, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, _e_ipc_cb_server_del, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA, _e_ipc_cb_server_data, NULL);
   
   return 1;
}

static int
_e_ipc_cb_server_add(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Add *e;
   
   e = event;
   ecore_ipc_server_send(e->server, 
			 6/*E_IPC_DOMAIN_FM*/,
			 1/*hello*/, 
			 0, 0, 0, NULL, 0); /* send hello */
   return 1;
}

static int
_e_ipc_cb_server_del(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Del *e;
   
   e = event;
   /* quit now */
   ecore_main_loop_quit();
   return 1;
}

static int
_e_ipc_cb_server_data(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Data *e;
   
   e = event;
   if (e->major != 6/*E_IPC_DOMAIN_FM*/) return 1;
   switch (e->minor)
     {
      case 1: /* monitor dir (and implicitly list) */
	  {
	     E_Dir *ed, *ped = NULL;
	     DIR *dir;
	     Evas_List *l;
	     
	     /* look for any previous dir entries monitoring this dir */
	     for (l = _e_dirs; l; l = l->next)
	       {
		  E_Dir *ed;
	     
		  ed = l->data;
		  if ((ed->mon) && (!strcmp(ed->dir, e->data)))
		    {
		       /* found a previous dir - save it in ped */
		       ped = ed;
		       break;
		    }
	       }
	     /* open the dir to list */
	     dir = opendir(e->data);
	     if (!dir)
	       {
		  E_Dir ted;
		  
		  /* we can't open the dir - tell E the dir is deleted as
		   * we can't look in it */
		  memset(&ted, 0, sizeof(E_Dir));
		  ted.id = e->ref;
		  _e_file_mon_dir_del(&ted, e->data);
	       }
	     else
	       {
		  Evas_List *files = NULL;
		  struct dirent *dp;
		  int dot_order = 0;
		  char buf[4096];
		  FILE *f;
		  
		  /* create a new dir entry */
		  ed = calloc(1, sizeof(E_Dir));
		  ed->id = e->ref;
		  ed->dir = evas_stringshare_add(e->data);
		  if (!ped)
		    {
		       /* if no previous monitoring dir exists - this one 
			* becomes the master monitor enty */
		       ed->mon = ecore_file_monitor_add(ed->dir, _e_cb_file_monitor, ed);
		       ed->mon_ref = 1;
		    }
		  else
		    {
		       /* an existing monitor exists - ref it up */
		       ed->mon_real = ped;
		       ped->mon_ref++;
		    }
		  _e_dirs = evas_list_append(_e_dirs, ed);
		  
		  /* read everything except a .order, . and .. */
		  while ((dp = readdir(dir)))
		    {
		       if ((!strcmp(dp->d_name, ".")) || (!strcmp(dp->d_name, "..")))
			 continue;
		       if (!strcmp(dp->d_name, ".order")) dot_order = 1;
		       files = evas_list_append(files, strdup(dp->d_name));
		    }
		  closedir(dir);
		  /* if there was a .order - we need to parse it */
		  if (dot_order)
		    {
		       snprintf(buf, sizeof(buf), "%s/.order", (char *)e->data);
		       f = fopen(buf, "r");
		       if (f)
			 {
			    Evas_List *f2 = NULL;
			    int len;
			    char *s;
			    
			    /* inset files in order if the existed in file 
			     * list before */
			    while (fgets(buf, sizeof(buf), f))
			      {
				 len = strlen(buf);
				 if (len > 0) buf[len - 1] = 0;
				 s = _e_str_list_remove(&files, buf);
				 if (s) f2 = evas_list_append(f2, s);
			      }
			    fclose(f);
			    /* append whats left */
			    while (files)
			      {
				 f2 = evas_list_append(f2, files->data);
				 files = evas_list_remove_list(files, files);
			      }
			    files = f2;
			 }
		    }
		  ed->fq = files;
		  /* FIXME: if .order file- load it, sort all items int it
		   * that are in files then just append whatever is left in
		   * alphabetical order
		   */
		  /* FIXME: maybe one day we can sort files here and handle
		   * .order file stuff here - but not today
		   */
		  /* note that we had a .order at all */
		  ed->dot_order = dot_order;
		  if (dot_order)
		    {
		       /* if we did - tell the E about this FIRST - it will
			* decide what to do if it first sees a .order or not */
		       if (!strcmp(e->data, "/"))
			 snprintf(buf, sizeof(buf), "/.order");
		       else
			 snprintf(buf, sizeof(buf), "%s/.order", (char *)e->data);
		       if (evas_list_count(files) == 1)
			 _e_file_add(ed, buf, 2);
		       else
			 _e_file_add(ed, buf, 1);
		    }
		  /* send empty file - indicate empty dir */
		  if (!files) _e_file_add(ed, "", 2);
		  /* and in an idler - list files, statting them etc. */
		  ed->idler = ecore_idler_add(_e_cb_file_mon_list_idler, ed);
		  ed->sync_num = DEF_SYNC_NUM;
	       }
	  }
	break;
      case 2: /* monitor dir end */
	  {
	     Evas_List *l;
	     
	     for (l = _e_dirs; l; l = l->next)
	       {
		  E_Dir *ed;
	     
		  /* look for the dire entry to stop monitoring */
		  ed = l->data;
		  if ((e->ref == ed->id) && (!strcmp(ed->dir, e->data)))
		    {
		       /* if this is not the real monitoring node - unref the
			* real one */
		       if (ed->mon_real)
			 {
			    /* unref original monitor node */
			    ed->mon_real->mon_ref--;
			    if (ed->mon_real->mon_ref == 0)
			      {
				 /* original is at 0 ref - free it */
				 _e_dir_del(ed->mon_real);
				 ed->mon_real = NULL;
			      }
			    /* free this node */
			    _e_dir_del(ed);
			 }
		       /* this is a core monitoring node - remove ref */
		       else
			 {
			    ed->mon_ref--;
			    /* we are the last ref - free */
			    if (ed->mon_ref == 0) _e_dir_del(ed);
			 }
		       /* remove from dirs list anyway */
		       _e_dirs = evas_list_remove_list(_e_dirs, l);
		       break;
		    }
	       }
	  }
	break;
      case 3: /* fop delete file/dir */
	  {
	     E_Fop *fop;
	     
	     fop = calloc(1, sizeof(E_Fop));
	     if (fop)
	       {
		  fop->id = e->ref;
		  fop->src = evas_stringshare_add(e->data);
		  _e_fops = evas_list_append(_e_fops, fop);
		  fop->idler = ecore_idler_add(_e_cb_fop_rm_idler, fop);
	       }
	  }
	break;
      case 4: /* fop trash file/dir */
	  {
	     E_Fop *fop;
	     
	     fop = calloc(1, sizeof(E_Fop));
	     if (fop)
	       {
		  fop->id = e->ref;
		  fop->src = evas_stringshare_add(e->data);
		  _e_fops = evas_list_append(_e_fops, fop);
		  fop->idler = ecore_idler_add(_e_cb_fop_trash_idler, fop);
	       }
	  }
	break;
      case 5: /* fop rename file/dir */
	  {
	     const char *src, *dst;
	     
	     src = e->data;
	     dst = src + strlen(src) + 1;
	     ecore_file_mv(src, dst);
	     /* FIXME: send back if succeeded or failed - why */
	     _e_path_fix_order(dst, ecore_file_get_file(src), 2, -9999, -9999);
	  }
	break;
      case 6: /* fop mv file/dir */
	  {
	     E_Fop *fop;
	     
	     fop = calloc(1, sizeof(E_Fop));
	     if (fop)
	       {
		  const char *src, *dst, *rel;
		  int rel_to, x, y;
		  
		  src = e->data;
		  dst = src + strlen(src) + 1;
		  rel = dst + strlen(dst) + 1;
		  memcpy(&rel_to, rel + strlen(rel) + 1, sizeof(int));
		  memcpy(&x, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
		  memcpy(&y, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
		  fop->id = e->ref;
		  fop->src = evas_stringshare_add(src);
		  fop->dst = evas_stringshare_add(dst);
		  fop->rel = evas_stringshare_add(rel);
		  fop->rel_to = rel_to;
		  fop->x = x;
		  fop->y = y;
		  _e_fops = evas_list_append(_e_fops, fop);
		  fop->idler = ecore_idler_add(_e_cb_fop_mv_idler, fop);
	       }
	  }
	break;
      case 7: /* fop cp file/dir */
	  {
	     E_Fop *fop;
	     
	     fop = calloc(1, sizeof(E_Fop));
	     if (fop)
	       {
		  const char *src, *dst, *rel;
		  int rel_to, x, y;
		  
		  src = e->data;
		  dst = src + strlen(src) + 1;
		  rel = dst + strlen(dst) + 1;
		  memcpy(&rel_to, rel + strlen(rel) + 1, sizeof(int));
		  memcpy(&x, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
		  memcpy(&y, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
		  fop->id = e->ref;
		  fop->src = evas_stringshare_add(src);
		  fop->dst = evas_stringshare_add(dst);
		  fop->rel = evas_stringshare_add(rel);
		  fop->rel_to = rel_to;
		  fop->x = x;
		  fop->y = y;
		  _e_fops = evas_list_append(_e_fops, fop);
		  fop->idler = ecore_idler_add(_e_cb_fop_cp_idler, fop);
	       }
	  }
	break;
      case 8: /* fop mkdir */
	  {
	     const char *src, *rel;
	     int rel_to, x, y;
	     
	     src = e->data;
	     rel = src + strlen(src) + 1;
	     memcpy(&rel_to, rel + strlen(rel) + 1, sizeof(int));
	     memcpy(&x, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
	     memcpy(&y, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
	     ecore_file_mkdir(src);
	     /* FIXME: send back if succeeded or failed - why */
	     _e_path_fix_order(src, rel, rel_to, x, y);
	  }
	break;
      case 9: /* fop mount fs */
	/* FIXME: implement later - for now in e */
	break;
      case 10: /* fop umount fs */
	/* FIXME: implement later - for now in e */
	break;
      case 11: /* quit */
	ecore_main_loop_quit();
	break;
      case 12: /* mon list sync */
	  {
	     Evas_List *l;
	     double stime;
	     
             for (l = _e_dirs; l; l = l->next)
	       {
		  E_Dir *ed;
		  
		  ed = l->data;
		  if (ed->fq)
		    {
		       if (ed->sync == e->response)
			 {
			    stime = ecore_time_get() - ed->sync_time;
			    /* try keep round trips to round trip tolerance */
			    if 
			      (stime < (DEF_ROUND_TRIP - DEF_ROUND_TRIP_TOLERANCE))
			      ed->sync_num += 1;
			    else if
			      (stime > (DEF_ROUND_TRIP + DEF_ROUND_TRIP_TOLERANCE))
			      ed->sync_num -= 1;
			    /* always sync at least 1 file */
			    if (ed->sync_num < 1) ed->sync_num = 1;
			    ed->idler = ecore_idler_add(_e_cb_file_mon_list_idler, ed);
			    break;
			 }
		    }
	       }
	  }
	break;
      case 13: /* dop ln -s */
	  {
	     const char *src, *dst, *rel;
	     int rel_to, x, y;
	     
	     src = e->data;
	     dst = src + strlen(src) + 1;
	     rel = dst + strlen(dst) + 1;
	     memcpy(&rel_to, rel + strlen(rel) + 1, sizeof(int));
	     memcpy(&x, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
	     memcpy(&y, rel + strlen(rel) + 1 + sizeof(int), sizeof(int));
	     ecore_file_symlink(src, dst);
             /* FIXME: send back file add if succeeded */
	  }
	break;
      default:
	break;
     }
   /* always send back an "OK" for each request so e can basically keep a
    * count of outstanding requests - maybe for balancing between fm
    * slaves later. ref_to is set to the the ref id in the request to 
    * allow for async handling later */
   ecore_ipc_server_send(_e_ipc_server,
			 6/*E_IPC_DOMAIN_FM*/,
			 2/*req ok*/,
			 0, e->ref, 0, NULL, 0);
   return 1;
}

static void
_e_cb_file_monitor(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
   E_Dir *ed;
   char *dir, *rp, *drp;
   const char *file;
   Evas_List *l;

   dir = ecore_file_get_dir(path);
   file = ecore_file_get_file(path);
   /* FIXME: get no create events if dir is empty */
   if ((event == ECORE_FILE_EVENT_CREATED_FILE) ||
       (event == ECORE_FILE_EVENT_CREATED_DIRECTORY))
     {
	rp = ecore_file_realpath(dir);
	for (l = _e_dirs; l; l = l->next)
	  {
	     ed = l->data;
	     drp = ecore_file_realpath(ed->dir);
	     if (!strcmp(rp, drp))
	       _e_file_add(ed, path, 0);
	     free(drp);
	  }
	free(rp);
     }
   else if ((event == ECORE_FILE_EVENT_DELETED_FILE) ||
	    (event == ECORE_FILE_EVENT_DELETED_DIRECTORY))
     {
	rp = ecore_file_realpath(dir);
	for (l = _e_dirs; l; l = l->next)
	  {
	     ed = l->data;
	     drp = ecore_file_realpath(ed->dir);
	     if (!strcmp(rp, drp))
	       _e_file_del(ed, path);
	  }
	free(rp);
     }
   else if (event == ECORE_FILE_EVENT_MODIFIED)
     {
	rp = ecore_file_realpath(dir);
	for (l = _e_dirs; l; l = l->next)
	  {
	     ed = l->data;
	     drp = ecore_file_realpath(ed->dir);
	     if (!strcmp(rp, drp))
	       _e_file_mod(ed, path);
	  }
	free(rp);
     }
   else if (event == ECORE_FILE_EVENT_DELETED_SELF)
     {
	rp = ecore_file_realpath(path);
	for (l = _e_dirs; l; l = l->next)
	  {
	     ed = l->data;
	     drp = ecore_file_realpath(ed->dir);
	     if (!strcmp(rp, drp))
	       _e_file_mon_dir_del(ed, path);
	  }
	free(rp);
     }
   free(dir);
}

static int
_e_cb_recent_clean(void *data)
{
   E_Dir *ed;
   Evas_List *l, *pl;
   E_Mod *m;
   double t_now;
   
   ed = data;
   ed->cleaning = 1;
   t_now = ecore_time_get();
   for (l = ed->recent_mods; l;)
     {
	m = l->data;
	pl = l;
	l = l->next;
	if ((m->mod) && ((t_now - m->timestamp) >= DEF_MOD_BACKOFF))
	  {
	     ed->recent_mods = evas_list_remove_list(ed->recent_mods, pl);
	     if (!m->done) _e_file_add_mod(ed, m->path, 5, 0);
	     evas_stringshare_del(m->path);
	     free(m);
	  }
     }
   ed->cleaning = 0;
   if (ed->recent_mods) return 1;
   ed->recent_clean = NULL;
   return 0;
}
			       

static void
_e_file_add_mod(E_Dir *ed, const char *path, int op, int listing)
{
   struct stat st;
   char *lnk = NULL, *rlnk = NULL;
   int broken_lnk = 0;
   int bsz = 0;
   unsigned char *p, buf
     /* file add/change format is as follows:
      * 
      * stat_info[stat size] + broken_link[1] + path[n]\0 + lnk[n]\0 + rlnk[n]\0 */
     [sizeof(struct stat) + 1 + 4096 + 4096 + 4096];

   /* FIXME: handle BACKOFF */
   if ((!listing) && (op == 5) && (!ed->cleaning)) /* 5 == mod */
     {
	Evas_List *l;
	E_Mod *m;
	double t_now;
	int skip = 0;
	
	t_now = ecore_time_get();
	for (l = ed->recent_mods; l; l = l->next)
	  {
	     m = l->data;
	     if ((m->mod) && (!strcmp(m->path, path)))
	       {
		  if ((t_now - m->timestamp) < DEF_MOD_BACKOFF)
		    {
		       m->done = 0;
		       skip = 1;
		    }
	       }
	  }
	if (!skip)
	  {
	     m = calloc(1, sizeof(E_Mod));
	     m->path = evas_stringshare_add(path);
	     m->mod = 1;
	     m->done = 1;
	     m->timestamp = t_now;
	     ed->recent_mods = evas_list_append(ed->recent_mods, m);
	  }
	if ((!ed->recent_clean) && (ed->recent_mods))
	  ed->recent_clean = ecore_timer_add(DEF_MOD_BACKOFF, _e_cb_recent_clean, ed);
	if (skip)
	  {
//	     printf("SKIP MOD %s %3.3f\n", path, t_now);
	     return;
	  }
     }
//   printf("MOD %s %3.3f\n", path, ecore_time_get());
   lnk = ecore_file_readlink(path);
   if (stat(path, &st) == -1)
     {
	if ((path[0] == 0) || (lnk)) broken_lnk = 1;
	else return;
     }
   if ((lnk) && (lnk[0] != '/')) rlnk = ecore_file_realpath(path);
   else if (lnk) rlnk = strdup(lnk);
   if (!lnk) lnk = strdup("");
   if (!rlnk) rlnk = strdup("");

   p = buf;
   /* NOTE: i am NOT converting this data to portable arch/os independant
    * format. i am ASSUMING e_fm_main and e are local and built together
    * and thus this will work. if this ever changes this here needs to
    * change */
   memcpy(buf, &st, sizeof(struct stat));
   p += sizeof(struct stat);
   
   p[0] = broken_lnk;
   p += 1;
   
   strcpy((char *)p, path);
   p += strlen(path) + 1;
   
   strcpy((char *)p, lnk);
   p += strlen(lnk) + 1;
   
   strcpy((char *)p, rlnk);
   p += strlen(rlnk) + 1;
   
   bsz = p - buf;
   ecore_ipc_server_send(_e_ipc_server, 6/*E_IPC_DOMAIN_FM*/, op, 0, ed->id,
			 listing, buf, bsz);
   if (lnk) free(lnk);
   if (rlnk) free(rlnk);
}

static void
_e_file_add(E_Dir *ed, const char *path, int listing)
{
   if (!listing)
     {
	/* FIXME: handle BACKOFF */
     }
   _e_file_add_mod(ed, path, 3, listing);/*file add*/
}

static void
_e_file_del(E_Dir *ed, const char *path)
{
     {
	/* FIXME: handle BACKOFF */
     }
   ecore_ipc_server_send(_e_ipc_server,
			 6/*E_IPC_DOMAIN_FM*/,
			 4/*file del*/,
			 0, ed->id, 0, (void *)path, strlen(path) + 1);
}

static void
_e_file_mod(E_Dir *ed, const char *path)
{
     {
	/* FIXME: handle BACKOFF */
     }
   _e_file_add_mod(ed, path, 5, 0);/*file change*/
}

static void
_e_file_mon_dir_del(E_Dir *ed, const char *path)
{
   ecore_ipc_server_send(_e_ipc_server,
			 6/*E_IPC_DOMAIN_FM*/,
			 6/*mon dir del*/,
			 0, ed->id, 0, (void *)path, strlen(path) + 1);
}

static void
_e_file_mon_list_sync(E_Dir *ed)
{
   _e_sync_num++;
   if (_e_sync_num == 0) _e_sync_num = 1;
   ed->sync = _e_sync_num;
   ed->sync_time = ecore_time_get();
   ecore_ipc_server_send(_e_ipc_server,
			 6/*E_IPC_DOMAIN_FM*/,
			 7/*mon list sync*/,
			 0, ed->id, ed->sync, NULL, 0);
}

static int
_e_cb_file_mon_list_idler(void *data)
{
   E_Dir *ed;
   int n = 0;
   char *file, buf[4096];
   
   ed = data;
   /* FIXME: spool off files in idlers and handle sync req's */
   while (ed->fq)
     {
	file = ed->fq->data;
	if (!((ed->dot_order) && (!strcmp(file, ".order"))))
	  {
	     if (!strcmp(ed->dir, "/"))
	       snprintf(buf, sizeof(buf), "/%s", file);
	     else
	       snprintf(buf, sizeof(buf), "%s/%s", ed->dir, file);
	     if ((!ed->fq->next) ||
		 ((!strcmp(ed->fq->next->data, ".order")) &&
		  (!ed->fq->next->next)))
	       _e_file_add(ed, buf, 2);
	     else
	       _e_file_add(ed, buf, 1);
	  }
	free(file);
	ed->fq = evas_list_remove_list(ed->fq, ed->fq);
	n++;
	if (n == ed->sync_num)
	  {
	     _e_file_mon_list_sync(ed);
	     ed->idler = NULL;
	     return 0;
	  }
     }
   ed->sync_num = DEF_SYNC_NUM;
   ed->sync = 0;
   ed->sync_time = 0.0;
   ed->idler = NULL;
   return 0;
}

static int
_e_cb_fop_rm_idler(void *data)
{
   E_Fop *fop;
   struct Fop_Data {
      DIR *dir;
      const char *path;
   } *fd, *fd2;
   struct dirent *dp = NULL;
   char buf[PATH_MAX], *lnk;
   
   fop = (E_Fop *)data;
   if (!fop->data)
     {
	snprintf(buf, sizeof(buf), "%s", fop->src);
	lnk = ecore_file_readlink(buf);
	if (!lnk)
	  {
	     if (ecore_file_is_dir(buf))
	       {
		  fd = calloc(1, sizeof(struct Fop_Data));
		  if (fd)
		    {
		       fop->data = evas_list_prepend(fop->data, fd);
		       fd->path = evas_stringshare_add(fop->src);
		       fd->dir = opendir(fd->path);
		    }
	       }
	     else
	       {
		  ecore_file_unlink(buf); /* FIXME: handle err */
	       }
	  }
	else
	  {
	     ecore_file_unlink(buf); /* FIXME: handle err */
	     free(lnk);
	  }
	
     }
   fd = evas_list_data(fop->data);
   if (!fd) goto stop;
   if (fd->dir) dp = readdir(fd->dir);
   else
     {
	/* FIXME: handle err  - if fd->diir is not a dir */
     }
   if (dp)
     {
	if (!((!strcmp(dp->d_name, ".")) || (!strcmp(dp->d_name, ".."))))
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", fd->path, dp->d_name);
	     lnk = ecore_file_readlink(buf);
	     if (!lnk)
	       {
		  if (ecore_file_is_dir(buf))
		    {
		       fd2 = calloc(1, sizeof(struct Fop_Data));
		       if (fd2)
			 {
			    fop->data = evas_list_prepend(fop->data, fd2);
			    fd2->path = evas_stringshare_add(buf);
			    fd2->dir = opendir(fd2->path);
			 }
		    }
		  else
		    {
		       ecore_file_unlink(buf); /* FIXME: handle err */
		    }
	       }
	     else
	       {
		  ecore_file_unlink(buf); /* FIXME: handle err */
		  free(lnk);
	       }
	  }
     }
   else
     {
	if (fd->dir) closedir(fd->dir);
	rmdir(fd->path); /* FIXME: handle err */
	evas_stringshare_del(fd->path);
	free(fd);
	fop->data = evas_list_remove(fop->data, fd);
	if (!fop->data) goto stop;
     }
   return 1;
   stop:
   evas_stringshare_del(fop->src);
   free(fop);
   _e_fops = evas_list_remove(_e_fops, fop);
   return 0;
   /* FIXME: send back if succeeded or failed - why */
}

static int
_e_cb_fop_trash_idler(void *data)
{
   /* FIXME: for now trash == rm - later move to trash */
   return _e_cb_fop_rm_idler(data);
}

static int
_e_cb_fop_mv_idler(void *data)
{
   E_Fop *fop;

   fop = (E_Fop *)data;
   if (!fop->data)
     {
	if (rename(fop->src, fop->dst) != 0)
	  {
	     if (errno == EXDEV)
	       {
		  /* copy it instead - but del after cp */
		  fop->idler = ecore_idler_add(_e_cb_fop_cp_idler, fop);
		  fop->del_after = 1;
		  return 0;
	       }
	     else
	       {
		  /* FIXME: handle error */
	       }
	  }
	_e_path_fix_order(fop->dst, fop->rel, fop->rel_to, fop->x, fop->y);
     }
   evas_stringshare_del(fop->src);
   evas_stringshare_del(fop->dst);
   free(fop);
   _e_fops = evas_list_remove(_e_fops, fop);
   return 0;
   /* FIXME: send back if succeeded or failed - why */
}

static int
_e_cb_fop_cp_idler(void *data)
{
   E_Fop *fop;
   struct Fop_Data {
      DIR *dir;
      const char *path, *path2;
   } *fd, *fd2;
   struct stat st;
   struct utimbuf ut;
   struct dirent *dp = NULL;
   char buf[PATH_MAX], buf2[PATH_MAX], *lnk;
   
   fop = (E_Fop *)data;
   if (!fop->data)
     {
	fd = calloc(1, sizeof(struct Fop_Data));
	if (fd)
	  {
	     fop->data = evas_list_append(fop->data, fd);
	     fd->path = evas_stringshare_add(fop->src);
	     fd->path2 = evas_stringshare_add(fop->dst);
	     fd->dir = opendir(fd->path);
	     snprintf(buf, sizeof(buf), "%s", fd->path);
	     snprintf(buf2, sizeof(buf2), "%s", fd->path2);
	     if (!lnk)
	       {
		  if (ecore_file_is_dir(buf))
		    {
		       if (stat(buf, &st) == 0)
			 {
			    /* mkdir at the other end - retain stat info */
			    if (!ecore_file_mkdir(buf2))
			      fop->gone_bad = 1;
			    chmod(buf2, st.st_mode);
			    chown(buf2, st.st_uid, st.st_gid);
			    ut.actime = st.st_atime;
			    ut.modtime = st.st_mtime;
			    utime(buf2, &ut);
			 }
		    }
		  else
		    {
		       if (stat(buf, &st) == 0)
			 {
			    if (S_ISFIFO(st.st_mode))
			      {
				 /* create fifo at other end */
				 if (mkfifo(buf2, st.st_mode) != 0)
				   fop->gone_bad = 1;
			      }
			    else if (S_ISREG(st.st_mode))
			      {
				 /* copy file data - retain file mode and stat data */
				 if (!ecore_file_cp(buf, buf2)) /* FIXME: this should be split up into the fop idler to do in idle time maybe 1 block or page at a time */
				   fop->gone_bad = 1;
			      }
			    chmod(buf2, st.st_mode);
			    chown(buf2, st.st_uid, st.st_gid);
			    ut.actime = st.st_atime;
			    ut.modtime = st.st_mtime;
			    utime(buf2, &ut);
			 }
		    }
	       }
	     else
	       {
		  if (stat(buf, &st) == 0)
		    {
		       /* duplicate link - retain stat data */
		       if (symlink(lnk, buf2) != 0)
			 fop->gone_bad = 1;
		       chmod(buf2, st.st_mode);
		       chown(buf2, st.st_uid, st.st_gid);
		       ut.actime = st.st_atime;
		       ut.modtime = st.st_mtime;
		       utime(buf2, &ut);
		    }
		  free(lnk);
	       }
	  }
	_e_path_fix_order(fop->dst, fop->rel, fop->rel_to, fop->x, fop->y);
     }
   fd = evas_list_data(evas_list_last(fop->data));
   if (!fd) goto stop;
   if (fd->dir) dp = readdir(fd->dir);
   else
     {
	/* FIXME: handle err */
     }
   if (dp)
     {
	if (!((!strcmp(dp->d_name, ".")) || (!strcmp(dp->d_name, ".."))))
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", fd->path, dp->d_name);
	     snprintf(buf2, sizeof(buf2), "%s/%s", fd->path2, dp->d_name);
	     lnk = ecore_file_readlink(buf);
	     if (!lnk)
	       {
		  if (ecore_file_is_dir(buf))
		    {
		       /* mkdir at the other end - retain stat info */
		       if (stat(buf, &st) == 0)
			 {
			    /* mkdir at the other end - retain stat info */
			    if (!ecore_file_mkdir(buf2))
			      fop->gone_bad = 1;
			    chmod(buf2, st.st_mode);
			    chown(buf2, st.st_uid, st.st_gid);
			    ut.actime = st.st_atime;
			    ut.modtime = st.st_mtime;
			    utime(buf2, &ut);
			 }
		       fd2 = calloc(1, sizeof(struct Fop_Data));
		       if (fd2)
			 {
			    fop->data = evas_list_append(fop->data, fd2);
			    fd2->path = evas_stringshare_add(buf);
			    fd2->path2 = evas_stringshare_add(buf2);
			    fd2->dir = opendir(fd2->path);
			 }
		    }
		  else
		    {
		       if (stat(buf, &st) == 0)
			 {
			    if (S_ISFIFO(st.st_mode))
			      {
				 /* create fifo at other end */
				 if (mkfifo(buf2, st.st_mode) != 0)
				   fop->gone_bad = 1;
				 /* FIXME: respect del_after flag */
			      }
			    else if (S_ISREG(st.st_mode))
			      {
				 /* copy file data - retain file mode and stat data */
				 if (!ecore_file_cp(buf, buf2)) /* FIXME: this should be split up into the fop idler to do in idle time maybe 1 block or page at a time */
				   fop->gone_bad = 1;
				 /* FIXME: respect del_after flag */
			      }
			    chmod(buf2, st.st_mode);
			    chown(buf2, st.st_uid, st.st_gid);
			    ut.actime = st.st_atime;
			    ut.modtime = st.st_mtime;
			    utime(buf2, &ut);
			    /* respect del_after flag */
			    if ((!fop->gone_bad) && (fop->del_after))
			      unlink(buf);
			 }
		    }
	       }
	     else
	       {
		  if (stat(buf, &st) == 0)
		    {
		       /* duplicate link - retain stat data */
		       if (symlink(lnk, buf2) != 0)
			 fop->gone_bad = 1;
		       chmod(buf2, st.st_mode);
		       chown(buf2, st.st_uid, st.st_gid);
		       ut.actime = st.st_atime;
		       ut.modtime = st.st_mtime;
		       utime(buf2, &ut);
		       /* respect del_after flag */
		       if ((!fop->gone_bad) && (fop->del_after))
			 unlink(buf);
		    }
		  free(lnk);
	       }
	  }
     }
   else
     {
	/* respect del_after flag */
	if ((!fop->gone_bad) && (fop->del_after))
	  unlink(fd->path);
	if (fd->dir) closedir(fd->dir);
	evas_stringshare_del(fd->path);
	evas_stringshare_del(fd->path2);
	free(fd);
	fop->data = evas_list_remove(fop->data, fd);
	if (!fop->data) goto stop;
	if (fop->gone_bad) goto stop;
     }
   return 1;
   
   stop:
   while (fop->data)
     {
	fd = evas_list_data(fop->data);
	if (fd)
	  {
	     if (fd->dir) closedir(fd->dir);
	     evas_stringshare_del(fd->path);
	     evas_stringshare_del(fd->path2);
	     free(fd);
	     fop->data = evas_list_remove(fop->data, fd);
	  }
     }
   evas_stringshare_del(fop->src);
   evas_stringshare_del(fop->dst);
   free(fop);
   _e_fops = evas_list_remove(_e_fops, fop);
   return 0;
   /* FIXME: send back if succeeded or failed - why */
}

static char *
_e_str_list_remove(Evas_List **list, char *str)
{
   Evas_List *l;
   
   for (l = *list; l; l = l->next)
     {
	char *s;
	
	s = l->data;
	if (!strcmp(s, str))
	  {
	     *list = evas_list_remove_list(*list, l);
	     return s;
	  }
     }
   return NULL;
}

static void
_e_path_fix_order(const char *path, const char *rel, int rel_to, int x, int y)
{
   char *d, buf[PATH_MAX];
   const char *f;
   
   if (!path) return;
   if (!rel[0]) return;
   f = ecore_file_get_file(path);
   if (!f) return;
   if (!strcmp(f, rel)) return;
   d = ecore_file_get_dir(path);
   if (!d) return;
   snprintf(buf, sizeof(buf), "%s/.order", d);
   if (ecore_file_exists(buf))
     {
	FILE *fh;
	Evas_List *files = NULL, *l;
	
	fh = fopen(buf, "r");
	if (fh)
	  {
	     int len;
	     
	     /* inset files in order if the existed in file 
	      * list before */
	     while (fgets(buf, sizeof(buf), fh))
	       {
		  len = strlen(buf);
		  if (len > 0) buf[len - 1] = 0;
		  files = evas_list_append(files, strdup(buf));
	       }
	     fclose(fh);
	  }
	/* remove dest file from .order - if there */
	for (l = files; l; l = l->next)
	  {
	     if (!strcmp(l->data, f))
	       {
		  free(l->data);
		  files = evas_list_remove_list(files, l);
		  break;
	       }
	  }
	/* now insert dest into list or replace entry */
	for (l = files; l; l = l->next)
	  {
	     if (!strcmp(l->data, rel))
	       {
		  if (rel_to == 2) /* replace */
		    {
		       free(l->data);
		       l->data = strdup(f);
		    }
		  else if (rel_to == 0) /* before */
		    {
		       files = evas_list_prepend_relative_list(files, strdup(f), l);
		    }
		  else if (rel_to == 1) /* after */
		    {
		       files = evas_list_append_relative_list(files, strdup(f), l);
		    }
		  break;
	       }
	  }
	snprintf(buf, sizeof(buf), "%s/.order", d);
	fh = fopen(buf, "w");
	if (fh)
	  {
	     while (files)
	       {
		  fprintf(fh, "%s\n", (char *)files->data);
		  free(files->data);
		  files = evas_list_remove_list(files, files);
	       }
	     fclose(fh);
	  }
     }
   free(d);
}

static void
_e_dir_del(E_Dir *ed)
{
   evas_stringshare_del(ed->dir);
   if (ed->idler) ecore_idler_del(ed->idler);
   if (ed->recent_clean)
     ecore_timer_del(ed->recent_clean);
   while (ed->recent_mods)
     {
	E_Mod *m;
	
	m = ed->recent_mods->data;
	evas_stringshare_del(m->path);
	free(m);
	ed->recent_mods = evas_list_remove_list(ed->recent_mods, ed->recent_mods);
     }
   while (ed->fq)
     {
	free(ed->fq->data);
	ed->fq = evas_list_remove_list(ed->fq, ed->fq);
     }
   free(ed);
}
