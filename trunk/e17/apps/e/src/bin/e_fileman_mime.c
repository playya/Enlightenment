/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/* e_fileman_mime.c
 * ================
 * will provide basic function for mime types, mime handlers
 * and operations with them, also a small app to actually 
 * edit the mime database
 *
 *
 * abstract types 
 * ==============================
 * the abstract types will be in a hierarchy way, all the leafs will heritate the parents
 * info (actions: run,open,edit,whatever) 
 *
 * properties:
 * mime: each leaf might have a mime type associated to it ? it wouldnt be neccesary for now, 
 * efm will handle only file names, in the future might visulize other file types?
 * 
 *
 * file 
 * +---- data
 *       +---- image
 *             +---- jpg
 *                   :mime image/jpeg
 *             +---- png
 *       +---- audio
 *             +---- mp3
 *             +---- ogg
 *             +---- wav
 * +---- directory
 * +---- special
 * +---- executable
 *       +---- a.out
 *       +---- java class
 *       +---- scripts
 *
 *
 *
 * file recognition for above types
 * ================================
 * 1. by regex 
 * 2. by suffix
 * 3. by type (directory,hidden,etc)
 * 4. properties (fifo,socket,executables,etc)
 *
 * icons visuals
 * =============
 * 1. by properties (read,write,execute)
 * 2. user defined (video directory, audo directory)
 * 3. by file contents (if a dir is full of mp3, might be an audio dir)
 *
 *
 *
 * actions
 * =======
 * each mime entry will have a list of possible actions to be taken for it
 * to define the command to execute there are several tokens like:
 *
 * %f => file input
 * %d => current directory
 * 
 *
 *
 *
 */

#include "e.h"

static char * _e_fm_mime_suffix_get(char *filename);
static void _e_fm_mime_action_append(E_Fm_Mime_Entry *entry, char *action_name);
static char *_e_fm_mime_action_tokenizer(Evas_List *files, E_Fm_Mime_Action *action);

static int init_count = 0;
static Evas_List *entries = NULL;
static Evas_List *actions = NULL;

/* returns the saved mime tree or a default one */   
int
e_fm_mime_init(void)
{
   E_Fm_Mime_Entry *root;
   E_Fm_Mime_Entry *entry;
   E_Fm_Mime_Entry *l1,*l2,*l3;
   E_Fm_Mime_Action *action;

   if(init_count)
     return 1;
   
   
   /* actions */
   /***********/
   action = E_NEW(E_Fm_Mime_Action,1);
   action->name = strdup("gimp_edit");
   action->label = strdup("Edit with Gimp");
   action->cmd = strdup("gimp %f");
   action->multiple = 1;
   actions = evas_list_append(actions,action);

   action = E_NEW(E_Fm_Mime_Action,1);
   action->name = strdup("xmms_enqueue");
   action->label = strdup("Add to XMMS Queue");
   action->cmd = strdup("xmms -e %f");
   action->multiple = 1;
   actions = evas_list_append(actions,action);
   
   action = E_NEW(E_Fm_Mime_Action,1);
   action->name = strdup("xmms_play");
   action->label = strdup("Play with XMMS");
   action->cmd = strdup("xmms %f");
   action->multiple = 0;
   actions = evas_list_append(actions,action);
   
   /* entries */
   /***********/
   root = E_NEW(E_Fm_Mime_Entry,1);
   root->name = strdup("file");
   root->label = strdup("Unkown File");
   root->level = 0;
   entries = evas_list_append(entries,root);
   /* data */
   entry = E_NEW(E_Fm_Mime_Entry,1);
   entry->name = strdup("data");
   entry->label = strdup("Data File");
   entry->parent = root;
   entry->level = 1;
   entries = evas_list_append(entries,entry);
   l1 = entry;
     {
	/* image */
	entry = E_NEW(E_Fm_Mime_Entry,1);
	entry->name = strdup("image");
	entry->label = strdup("Image File");
	entry->parent = l1;
	entry->level = 2;
	_e_fm_mime_action_append(entry, "gimp_edit");
	l2 = entry;
	  {
	     /* jpg */
	     entry = E_NEW(E_Fm_Mime_Entry, 1);
	     entry->name = strdup("jpg");
	     entry->label = strdup("JPEG Image");
	     entry->parent = l2;
	     entry->level = 3;
	     entry->suffix = strdup("jpg");
	     entries = evas_list_append(entries,entry);
	     
	     /* png */
	     entry = E_NEW(E_Fm_Mime_Entry, 1);
	     entry->name = strdup("png");
	     entry->label = strdup("PNG Image");
	     entry->parent = l2;
	     entry->level = 3;
	     entry->suffix = strdup("png");
	     entries = evas_list_append(entries,entry);

	  }
	/* audio */
	entry = E_NEW(E_Fm_Mime_Entry,1);
	entry->name = strdup("audio");
	entry->label = strdup("Audio File");
	entry->parent = l1;
	entry->level = 2;
	_e_fm_mime_action_append(entry, "xmms_play");
	_e_fm_mime_action_append(entry, "xmms_enqueue");
	l2 = entry;
	  {
	     /* mp3 */
	     entry = E_NEW(E_Fm_Mime_Entry, 1);
	     entry->name = strdup("mp3");
	     entry->label = strdup("MP3 Audio");
	     entry->parent = l2;
	     entry->level = 3;
	     entry->suffix = strdup("mp3");
	     entries = evas_list_append(entries,entry);
	     
	     /* ogg */
	     entry = E_NEW(E_Fm_Mime_Entry, 1);
	     entry->name = strdup("ogg");
	     entry->label = strdup("OGG Audio");
	     entry->parent = l2;
	     entry->level = 3;
	     entry->suffix = strdup("ogg");
	     entries = evas_list_append(entries,entry);

	     /* wav */
	     entry = E_NEW(E_Fm_Mime_Entry, 1);
	     entry->name = strdup("wav");
	     entry->label = strdup("WAV Audio");
	     entry->parent = l2;
	     entry->level = 3;
	     entry->suffix = strdup("wav");
	     entries = evas_list_append(entries,entry);
	  }
     }
   /* directory */
   entry = E_NEW(E_Fm_Mime_Entry,1);
   entry->name = strdup("folder");
   entry->label = strdup("Folder");
   entry->type = E_FM_FILE_TYPE_DIRECTORY;
   entry->parent = root;
   entry->level = 1;
   entries = evas_list_append(entries,entry);
   l1 = entry;


   
   init_count++;
   return 1;
}

void
e_fm_mime_shutdwon(void)
{
   init_count--;
}

/* returns the shortest root for both entries @e1 and @e2 
 * FIXME can be implemented faster?
 */
E_Fm_Mime_Entry *
e_fm_mime_common(E_Fm_Mime_Entry *e1, E_Fm_Mime_Entry *e2)
{

   E_Fm_Mime_Entry *tmp;
   int i;
   int count;
   
   /* take the lowest on the tree */
   /* set the e1 upper, e2 lower  */
   if(e1->level > e2->level)
     {
	count = e1->level - e2->level;
	tmp = e1;
	e1 = e2;
	e2 = tmp;
     }
   else
	count = e2->level - e1-> level;
	   
   /* first equal levels */
   for(i = 0; i < count; i++)
     {
	e2 = e2->parent;
     }
   /* get up on the tree until we find the same parent */
   for(i = e1->level; i >= 0; i--)
     {
	if(!strcmp(e1->name,e2->name))
	  return e1;
	e1 = e1->parent;
	e2 = e2->parent;
     }
   /* this should never happen */
   return NULL;
}

/* returns the shortest root of a list of mime entries @mis 
 * FIXME can be implemented faster? delete the list while iterating
 */
E_Fm_Mime_Entry *
e_fm_mime_list(Evas_List *mis)
{
   E_Fm_Mime_Entry *entry;
   Evas_List *l;

   entry = (E_Fm_Mime_Entry *)mis->data;
   for(l = mis->next; l; l = l->next)
     {
	E_Fm_Mime_Entry *eme;

	eme = (E_Fm_Mime_Entry *)l->data;
	entry = e_fm_mime_common(entry,eme);
     }
   return entry;
}


/* returns the mime entry for a file */
void 
e_fm_mime_set(E_Fm_File *file)
{
   Evas_List *l;
   for(l = entries; l; l = l->next)
     {
	E_Fm_Mime_Entry *entry;
	
	entry = (E_Fm_Mime_Entry *)l->data;
	/* FIXME add all the possible comparision, suffix,regexp,flags,etc */

	if(entry->suffix)
	  {
	     char *suffix;

	     suffix = _e_fm_mime_suffix_get(file->name);
	     if(!suffix)
	       continue;
	     if(!strcmp(suffix,entry->suffix))
	       {
		  printf("found by suffix %s\n", suffix);
		  file->mime = entry;
	       }
	     free(suffix);
	  }
	if(entry->type)
	  {
	     if(entry->type == file->type)
	       file->mime = entry;
	  }
     }
   /* if it doesnt match anything set to the root */
   if(!file->mime)
     file->mime = (E_Fm_Mime_Entry*)entries->data;
}

E_Fm_Mime_Action *
e_fm_mime_action_get_by_label(char *label)
{
   Evas_List *l;
   E_Fm_Mime_Action *action = NULL;
   
   for(l = actions; l; l = l->next)
     {
	action = (E_Fm_Mime_Action*)l->data;
	if(!strcmp(label,action->label))
	  break;
     }
   return action;
}

/* will call the command of an @action for the list of E_Fm_Files @files */
int
e_fm_mime_action_call(Evas_List *files, E_Fm_Mime_Action *action)
{
   Ecore_Exe *exe;
   char *command;

   /* FIXME: use the e app execution mechanisms where possible so we can
    * collect error output
    */
   command = _e_fm_mime_action_tokenizer(files,action);
   printf("going to execute %s\n", command);
   exe = ecore_exe_run(command, NULL);

   if (!exe)
     {
	e_error_dialog_show(_("Run Error"),
			    _("Enlightenment was unable to fork a child process:\n"
			      "\n"
			      "%s\n"
			      "\n"),
			    command);
	return 0;
     }
   return 1;
}


/* subsystem functions */
/***********************/
static char*
_e_fm_mime_action_tokenizer(Evas_List *files, E_Fm_Mime_Action *action)
{
   char *buf;
   char *c;
   int i, bsize,trans;
   Evas_List *l;

   buf = calloc(PATH_MAX,sizeof(char));
   bsize = PATH_MAX;
   i = 0;
   trans = 0;
   for(c = action->cmd; *c; c++)
     {
	if( i > bsize - 1)
	  {
	     bsize += PATH_MAX;
	     buf = realloc(buf,bsize);
	  }
	if(trans)
	  {
	     char *astr = NULL;
	     if(*c == 'f')
	       {
		  int i = 2;
		  char *f = NULL;

		  astr = calloc(PATH_MAX,sizeof(char));
		  for(l = files; l; l = l->next)
		    {
		       E_Fm_File *file;
		       
		       file = (E_Fm_File *)l->data;
		       sprintf(astr,"%s %s",astr,file->path);
		       astr = realloc(astr,PATH_MAX*i);
		       i++;
		    }
	       }
	     if(!astr)
	       continue;
	     if(bsize < i + strlen(astr))
	       {
		  bsize += strlen(astr) + 1;
		  buf = realloc(buf,bsize);
	       }
	     buf[i-1] = '\0';
	     sprintf(buf, "%s%s", buf, astr);
	     i += strlen(astr) - 1;
	     trans = 0;
	     free(astr);
	     continue;
	  }
	if(*c == '%')
	     trans = 1;
	else
	     buf[i] = *c;
	i++;
     }
   return buf;
}


static void
_e_fm_mime_action_append(E_Fm_Mime_Entry *entry, char *action_name)
{
   Evas_List *l;

   for(l = actions; l; l = l->next)
     {
	E_Fm_Mime_Action *action;

	action = (E_Fm_Mime_Action *)l->data;
	if(!strcmp(action->name, action_name))
	  {
	     entry->actions = evas_list_append(entry->actions, action);
	     break;
	  }
     }
}

static char *
_e_fm_mime_suffix_get(char *filename)
{
   char *suf;

   suf = strrchr(filename, '.');
   if (suf)
     {
	suf++;
	suf = strdup(suf);
	return suf;
     }
   return NULL;
}
