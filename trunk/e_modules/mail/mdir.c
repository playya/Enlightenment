#include <e.h>
#include "e_mod_main.h"
#include "mdir.h"

static Evas_List *mdirs;

static void _mail_mdir_check_mail(void *data, Ecore_File_Monitor *monitor, Ecore_File_Event event, const char *path); 
static int _mail_mdir_get_files(const char *path);

void
_mail_mdir_add_mailbox(void *data, void *data2) 
{
   MdirClient *mc;
   Config_Box *cb;
   
   cb = data2;
   if (!cb) return;
   
   mc = E_NEW(MdirClient, 1);
   mc->config = cb;
   mc->data = data;
   mc->config->num_new = 0;
   mc->config->num_total = 0;
   mc->monitor = ecore_file_monitor_add(cb->new_path, _mail_mdir_check_mail, mc);

   mdirs = evas_list_append(mdirs, mc);
}

void
_mail_mdir_del_mailbox(void *data) 
{
   Config_Box *cb;
   Evas_List *l;
   
   cb = data;
   if (!cb) return;
   for (l = mdirs; l; l = l->next) 
     {
	MdirClient *mc;
	
	mc = l->data;
	if (!mc) continue;
	if (mc->config != cb) continue;
	mdirs = evas_list_remove(mdirs, mc);
	free(mc);
	break;
     }
}

void
_mail_mdir_shutdown() 
{
   if (!mdirs) return;
   while (mdirs) 
     {
	MdirClient *mc;
	
	mc = mdirs->data;
	if (mc->monitor)
	  ecore_file_monitor_del(mc->monitor);
	mdirs = evas_list_remove_list(mdirs, mdirs);
	free(mc);
     }
}

/* PRIVATES */
static void 
_mail_mdir_check_mail(void *data, Ecore_File_Monitor *monitor, Ecore_File_Event event, const char *path) 
{
   MdirClient *mc;
   Instance *inst;
   
   mc = data;
   if (!mc) return;

   mc->config->num_total = _mail_mdir_get_files(mc->config->cur_path);
   mc->config->num_new = _mail_mdir_get_files(mc->config->new_path);
   inst = mc->data;
   inst->count = mc->config->num_new;
   _mail_set_text(inst);
   if ((mc->config->num_new > 0) && (mc->config->use_exec) && (mc->config->exec))
     _mail_start_exe(mc->config);
}

static int 
_mail_mdir_get_files(const char *path) 
{
   Ecore_List *l;
   char *item;
   int i = 0;
   
   l = ecore_file_ls(path);
   ecore_list_goto_first(l);
   while ((item = (char *)ecore_list_next(l)) != NULL) 
     {
	if ((!strcmp(item, ".")) || (!strcmp(item, ".."))) continue;
	i++;
     }
   ecore_list_destroy(l);
   return i;
}
