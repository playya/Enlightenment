/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

#ifdef EFM_DEBUG
# define D(x)  do {printf(__FILE__ ":%d:  ", __LINE__); printf x; fflush(stdout);} while (0)
#else
# define D(x)  ((void) 0)
#endif

typedef struct _E_Fm_Assoc_App              E_Fm_Assoc_App;
struct _E_Fm_Assoc_App
{
   char *mime;
   char *app;
};

/* local subsystem functions */
static void _e_fm_file_free(E_Fm_File *file);

/* TODO Make init and shutdown func that populates the assoc */
static E_Config_DD *assoc_app_edd;
static Evas_List   *assoc_apps;

#if 0
	{
	   E_Fm_Assoc_App *assoc;

	   assoc = E_NEW(E_Fm_Assoc_App, 1);
	   assoc->mime = (char*)E_NEW(char *, 5);
	   snprintf(assoc->mime, 5, "%s", ".jpg");
	   assoc->app = (char*)E_NEW(char *, 7);
	   snprintf(assoc->app, 7, "gqview");
	   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

	   assoc = E_NEW(E_Fm_Assoc_App, 1);
	   assoc->mime = (char*)E_NEW(char *, 5);
	   snprintf(assoc->mime, 5, "%s", ".png");
	   assoc->app = (char*)E_NEW(char *, 7);
	   snprintf(assoc->app, 7, "gqview");
	   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);
	}
#endif

/* externally accessible functions */
E_Fm_File *
e_fm_file_new(const char *filename)
{
   E_Fm_File *file;
   struct stat st;

   if (stat(filename, &st) == -1) return NULL;

   file = E_OBJECT_ALLOC(E_Fm_File, E_FM_FILE_TYPE, _e_fm_file_free);
   if (!file) return NULL;
   file->path = strdup(filename);
   if (!file->path) goto error;
   file->name = strdup(ecore_file_get_file(filename));
   if (!file->name) goto error;
   /* Get attributes */
   file->device = st.st_dev;
   file->inode = st.st_ino;
   file->mode = st.st_mode;
   file->nlink = st.st_nlink;
   file->owner = st.st_uid;
   file->group = st.st_gid;
   file->rdev = st.st_rdev;
   file->size = st.st_size;
   file->atime = st.st_atime;
   file->mtime = st.st_mtime;
   file->ctime = st.st_ctime;

   if (S_ISDIR(file->mode))
     file->type |= E_FM_FILE_TYPE_DIRECTORY;
   else if (S_ISREG(file->mode))
     file->type = E_FM_FILE_TYPE_FILE;
   else if (S_ISLNK(file->mode))
     file->type = E_FM_FILE_TYPE_SYMLINK;
   else
     file->type = E_FM_FILE_TYPE_UNKNOWN;

   if (file->name[0] == '.')
     file->type |= E_FM_FILE_TYPE_HIDDEN;

   file->preview_funcs = E_NEW(E_Fm_File_Preview_Function, 4);
   file->preview_funcs[0] = e_fm_file_is_image;
   file->preview_funcs[1] = e_fm_file_is_etheme;
   file->preview_funcs[2] = e_fm_file_is_ebg;
   file->preview_funcs[3] = e_fm_file_is_eap;

   D(("e_fm_file_new: %s\n", filename));
   return file;

error:
   if (file->path) free(file->path);
   if (file->name) free(file->name);
   free(file);
   return NULL;
}

int
e_fm_file_rename(E_Fm_File *file, const char *name)
{
   char path[PATH_MAX], *dir;

   if ((!name) || (!name[0])) return 0;

   dir = ecore_file_get_dir(file->path);
   if (!dir) return 0;
   snprintf(path, sizeof(path), "%s/%s", dir, name);

   if (ecore_file_mv(file->path, path))
     {
	free(file->path);
	file->path = strdup(path);
	free(file->name);
	file->name = strdup(name);
	D(("e_fm_file_rename: ok (%p) (%s)\n", file, name));
	return 1;
     }
   else
     {
	D(("e_fm_file_rename: fail (%p) (%s)\n", file, name));
	return 0;
     }
}

int
e_fm_file_delete(E_Fm_File *file)
{
   if (ecore_file_recursive_rm(file->path))
     {
	free(file->path);
	file->path = NULL;
	free(file->name);
	file->name = NULL;
	D(("e_fm_file_delete: ok (%p) (%s)\n", file, file->name));
	return 1;
     }
   else
     {
	D(("e_fm_file_delete: fail (%p) (%s)\n", file, file->name));
	return 0;
     }
}

int
e_fm_file_copy(E_Fm_File *file, const char *name)
{
   if ((!name) || (!name[0])) return 0;

   if (ecore_file_cp(file->path, name))
     {
	free(file->path);
	file->path = strdup(name);
	free(file->name);
	file->name = strdup(ecore_file_get_file(name));
	D(("e_fm_file_copy: ok (%p) (%s)\n", file, name));
	return 1;
     }
   else
     {
	D(("e_fm_file_copy: fail (%p) (%s)\n", file, name));
	return 0;
     }
}

int
e_fm_file_can_preview(E_Fm_File *file)
{
   int i;

   D(("e_fm_file_can_preview: (%s) (%p)\n", file->name, file));
   for (i = 0; i < sizeof(file->preview_funcs); i++)
     {
	E_Fm_File_Preview_Function func;
	func = file->preview_funcs[i];
	if (func(file))
	  return 1;
     }
   return 0;
}

int
e_fm_file_is_image(E_Fm_File *file)
{
   /* We need to check if it is a filetype supported by evas.
    * If it isn't supported by evas, we can't show it in the
    * canvas.
    */
   char *ext;

   if ((file->type != E_FM_FILE_TYPE_FILE) && (file->type != E_FM_FILE_TYPE_SYMLINK)) return 0;

   ext = strrchr(file->name, '.');
   if (!ext) return 0;

   D(("e_fm_file_is_image: (%p)\n", file));
   return (!strcasecmp(ext, ".jpg")) || (!strcasecmp(ext, ".png")) ||
	  (!strcasecmp(ext, ".jpeg"));
}

int
e_fm_file_is_etheme(E_Fm_File *file)
{
   int          val;
   char        *ext;
   Evas_List   *groups, *l;

   if ((file->type != E_FM_FILE_TYPE_FILE) && (file->type != E_FM_FILE_TYPE_SYMLINK)) return 0;

   ext = strrchr(file->name, '.');
   if (!ext) return 0;

   if (strcasecmp(ext, ".edj"))
     return 0;

   val = 0;
   groups = edje_file_collection_list(file->path);
   if (!groups)
     return 0;

   for (l = groups; l; l = l->next)
     {
	if (!strcmp(l->data, "widgets/border/default/border"))
	  {
	     val = 1;
	     break;
	  }
     }
   edje_file_collection_list_free(groups);
   return val;
}

int
e_fm_file_is_ebg(E_Fm_File *file)
{
   int          val;
   char        *ext;
   Evas_List   *groups, *l;

   if ((file->type != E_FM_FILE_TYPE_FILE) && (file->type != E_FM_FILE_TYPE_SYMLINK)) return 0;

   ext = strrchr(file->name, '.');
   if (!ext) return 0;

   if (strcasecmp(ext, ".edj"))
     return 0;

   val = 0;
   groups = edje_file_collection_list(file->path);
   if (!groups)
     return 0;

   for (l = groups; l; l = l->next)
     {
	if (!strcmp(l->data, "desktop/background"))
	  {
	     val = 1;
	     break;
	  }
     }
   edje_file_collection_list_free(groups);
   return val;
}

int
e_fm_file_is_eap(E_Fm_File *file)
{
   char *ext;
   E_App *app;

   if ((file->type != E_FM_FILE_TYPE_FILE) && (file->type != E_FM_FILE_TYPE_SYMLINK)) return 0;

   ext = strrchr(file->name, '.');
   if (!ext) return 0;

   if (strcasecmp(ext, ".eap"))
     return 0;

   app = e_app_new(file->path, 0);
   if (!app)
     {
	e_object_unref(E_OBJECT(app));
	return 0;
     }
   e_object_unref(E_OBJECT(app));
   return 1;
}

int
e_fm_file_can_exec(E_Fm_File *file)
{
   char *ext;

   ext = strrchr(file->name, '.');
   if (ext)
     {
	if (!strcasecmp(ext, ".eap"))
	  {
	     D(("e_fm_file_can_exec: true (%p) (%s)\n", file, file->name));
	     return 1;
	  }
     }

   if (ecore_file_can_exec(file->path))
     {
	D(("e_fm_file_can_exec: true (%p) (%s)\n", file, file->name));
	return 1;
     }

   D(("e_fm_file_can_exec: false (%p) (%s)\n", file, file->name));
   return 0;
}

int
e_fm_file_exec(E_Fm_File *file)
{
   Ecore_Exe *exe;
   char *ext;

   ext = strrchr(file->name, '.');
   if (ext)
     {
	if (!strcasecmp(ext, ".eap"))
	  {
	     E_App *e_app;
	     Ecore_Exe *exe;

	     e_app = e_app_new(file->path, 0);

	     if (!e_app) return 0;

	     exe = ecore_exe_run(e_app->exe, NULL);
	     if (exe) ecore_exe_free(exe);
	     e_object_unref(E_OBJECT(e_app));
	     D(("e_fm_file_exec: eap (%p) (%s)\n", file, file->name));
	     return 1;
	  }
     }

   exe = ecore_exe_run(file->path, NULL);
   if (!exe)
     {
	e_error_dialog_show(_("Run Error"),
			    _("Enlightenment was unable fork a child process:\n"
			      "\n"
			      "%s\n"
			      "\n"),
			    file->path);
	D(("e_fm_file_exec: fail (%p) (%s)\n", file, file->name));
	return 0;
     }
   /* E/app is the correct tag if the data is en E_App!
   ecore_exe_tag_set(exe, "E/app");
   */
   D(("e_fm_file_exec: ok (%p) (%s)\n", file, file->name));
   return 1;
}

int
e_fm_file_assoc_set(E_Fm_File *file, const char *assoc)
{
   /* TODO */
   return 1;
}

int
e_fm_file_assoc_exec(E_Fm_File *file)
{
   char app[PATH_MAX * 2];
   Evas_List *l;
   E_Fm_Assoc_App *assoc;
   Ecore_Exe *exe;

   for (l = assoc_apps; l; l = l->next)
     {
	char *ext;

	assoc = l->data;
	ext = strrchr(file->path, '.');
	if ((ext) && (!strcasecmp(ext, assoc->mime)))
	  break;
	assoc = NULL;
     }

   if (!assoc) return 0;

   snprintf(app, PATH_MAX * 2, "%s %s", assoc->app, file->path);
   exe = ecore_exe_run(app, NULL);

   if (!exe)
     {
	e_error_dialog_show(_("Run Error"),
			    _("Enlightenment was unable fork a child process:\n"
			      "\n"
			      "%s\n"
			      "\n"),
			    app);
	D(("e_fm_assoc_exec: fail (%s)\n", app));
	return 0;
     }
   /*
    * ecore_exe_tag_set(exe, "E/app");
    */
   D(("e_fm_assoc_exec: ok (%s)\n", app));
   return 1;
}

/* local subsystem functions */
static void
_e_fm_file_free(E_Fm_File *file)
{
   D(("_e_fm_file_free: (%p) (%s)\n", file, file->name));
   free(file->preview_funcs);
   if (file->path) free(file->path);
   if (file->name) free(file->name);
   free(file);
}

