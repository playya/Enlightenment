/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <ctype.h>
#include "ecore_file_private.h"


static int init = 0;

/* externally accessible functions */
EAPI int
ecore_file_init()
{
   if (++init != 1) return init;

   if (!ecore_file_monitor_init())
     goto error;
   if (!ecore_file_path_init())
     goto error;
   if (!ecore_file_download_init())
     goto error;
   return init;

error:

   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();
   ecore_file_download_shutdown();

   return --init;
}

EAPI int
ecore_file_shutdown()
{
   if (--init != 0) return init;

   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();
   ecore_file_download_shutdown();

   return init;
}

EAPI time_t
ecore_file_mod_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_mtime;
}

EAPI int
ecore_file_size(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_size;
}

EAPI int
ecore_file_exists(const char *file)
{
   struct stat st;

   /*Workaround so that "/" returns a true, otherwise we can't monitor "/" in ecore_file_monitor*/
   if (stat(file, &st) < 0 && strcmp(file, "/")) return 0;
   return 1;
}

EAPI int
ecore_file_is_dir(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

EAPI int
ecore_file_mkdir(const char *dir)
{
   if (mkdir(dir, default_mode) < 0) return 0;
   return 1;
}

EAPI int
ecore_file_rmdir(const char *dir)
{
   if (rmdir(dir) < 0) return 0;
   return 1;
}

EAPI int
ecore_file_unlink(const char *file)
{
   if (unlink(file) < 0) return 0;
   return 1;
}

EAPI int
ecore_file_recursive_rm(const char *dir)
{
   DIR                *dirp;
   struct dirent      *dp;
   int                ret;

   if (!ecore_file_is_dir(dir))
     return ecore_file_unlink(dir);

   dirp = opendir(dir);
   if (!dirp) return 0;

   ret = 1;
   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	  {
	     char path[PATH_MAX];
	     struct stat st;

	     snprintf(path, PATH_MAX, "%s/%s", dir, dp->d_name);
	     if (stat(path, &st) == -1)
	       {
		  ret = 0;
		  continue;
	       }

	     if (S_ISDIR(st.st_mode))
	       {
		  ecore_file_recursive_rm(path);
		  ecore_file_rmdir(path);
	       }
	     else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode))
	       {
		  ecore_file_unlink(path);
	       }
	  }
     }
   closedir(dirp);

   if (!ecore_file_rmdir(dir))
     ret = 0;

   return ret;
}

EAPI int
ecore_file_mkpath(const char *path)
{
   char ss[PATH_MAX];
   int  i;

   ss[0] = 0;
   i = 0;
   while (path[i])
     {
	if (i == sizeof(ss) - 1) return 0;
	ss[i] = path[i];
	ss[i + 1] = 0;
	if (path[i] == '/')
	  {
	     ss[i] = 0;
	     if ((ecore_file_exists(ss)) && (!ecore_file_is_dir(ss))) return 0;
	     else if (!ecore_file_exists(ss)) ecore_file_mkdir(ss);
	     ss[i] = '/';
	  }
	i++;
     }
   if ((ecore_file_exists(ss)) && (!ecore_file_is_dir(ss))) return 0;
   else if (!ecore_file_exists(ss)) ecore_file_mkdir(ss);
   return 1;
}

EAPI int
ecore_file_cp(const char *src, const char *dst)
{
   FILE               *f1, *f2;
   char                buf[16384];
   char                realpath1[PATH_MAX];
   char                realpath2[PATH_MAX];
   size_t              num;

   if (!realpath(src, realpath1)) return 0;
   if (realpath(dst, realpath2) && !strcmp(realpath1, realpath2)) return 0;

   f1 = fopen(src, "rb");
   if (!f1) return 0;
   f2 = fopen(dst, "wb");
   if (!f2)
     {
	fclose(f1);
	return 0;
     }
   while ((num = fread(buf, 1, sizeof(buf), f1)) > 0) fwrite(buf, 1, num, f2);
   fclose(f1);
   fclose(f2);
   return 1;
}

EAPI int
ecore_file_mv(const char *src, const char *dst)
{
   if (ecore_file_exists(dst)) return 0;
   if (rename(src, dst)) return 0;
   return 1;
}

EAPI int
ecore_file_symlink(const char *src, const char *dest)
{
   if(!symlink(src, dest))
     return 1;
   return 0;
}

EAPI char *
ecore_file_realpath(const char *file)
{
   char  buf[PATH_MAX];
   struct stat st;

   if (!realpath(file, buf) || stat(buf, &st)) return strdup("");
   return strdup(buf);
}

EAPI const char *
ecore_file_get_file(const char *path)
{
   char *result = NULL;

   if (!path) return NULL;
   if ((result = strrchr(path, '/'))) result++;
   else result = (char *)path;
   return result;
}

EAPI char *
ecore_file_get_dir(const char *file)
{
   char               *p;
   char                buf[PATH_MAX];

   strncpy(buf, file, PATH_MAX);
   p = strrchr(buf, '/');
   if (!p)
     {
	return strdup(file);
     }
   *p = 0;
   return strdup(buf);
}

EAPI int
ecore_file_can_read(const char *file)
{
   if (!file) return 0;
   if (!access(file, R_OK)) return 1;
   return 0;
}

EAPI int
ecore_file_can_write(const char *file)
{
   if (!file) return 0;
   if (!access(file, W_OK)) return 1;
   return 0;
}

EAPI int
ecore_file_can_exec(const char *file)
{
   if (!file) return 0;
   if (!access(file, X_OK)) return 1;
   return 0;
}

EAPI char *
ecore_file_readlink(const char *link)
{
   char                buf[PATH_MAX];
   int                 count;

   if ((count = readlink(link, buf, sizeof(buf))) < 0) return NULL;
   buf[count] = 0;
   return strdup(buf);
}

EAPI Ecore_List *
ecore_file_ls(const char *dir)
{
   char               *f;
   DIR                *dirp;
   struct dirent      *dp;
   Ecore_List         *list;
   Ecore_Sheap        *heap;

   dirp = opendir(dir);
   if (!dirp) return NULL;

   list = ecore_list_new();
   ecore_list_set_free_cb(list, free);

   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	  {
	       f = strdup(dp->d_name);
	       ecore_list_append(list, f);
	  }
     }
   closedir(dirp);

   /*
    * Push the data into a heap.
    */
   heap = ecore_sheap_new(ECORE_COMPARE_CB(strcasecmp), ecore_list_nodes(list));
   while ((f = ecore_list_remove_first(list)))
     {
	ecore_sheap_insert(heap, f);
     }

   /*
    * Extract in sorted order.
    */
   while ((f = ecore_sheap_extract(heap)))
     {
	ecore_list_append(list, f);
     }

   ecore_sheap_destroy(heap);

   ecore_list_goto_first(list);
   return list;
}

EAPI char *
ecore_file_app_exe_get(const char *app)
{
   char *p, *pp, *exe1 = NULL, *exe2 = NULL;
   char *exe = NULL;
   int in_quot_dbl = 0, in_quot_sing = 0, restart = 0;

   p = (char *)app;
restart:
   while ((*p) && (isspace(*p))) p++;
   exe1 = p;
   while (*p)
     {
	if (in_quot_sing)
	  {
	     if (*p == '\'')
	       in_quot_sing = 0;
	  }
	else if (in_quot_dbl)
	  {
	     if (*p == '\"')
	       in_quot_dbl = 0;
	  }
	else
	  {
	     if (*p == '\'')
	       in_quot_sing = 1;
	     else if (*p == '\"')
	       in_quot_dbl = 1;
	     if ((isspace(*p)) && (!((p > app) && (p[-1] != '\\'))))
	       break;
	  }
	p++;
     }
   exe2 = p;
   if (exe2 == exe1) return NULL;
   if (*exe1 == '~')
     {
	char *homedir;
	int len;

	/* Skip ~ */
	exe1++;

	homedir = getenv("HOME");
	if (!homedir) return NULL;
	len = strlen(homedir);
	if (exe) free(exe);
	exe = malloc(len + exe2 - exe1 + 2);
	if (!exe) return NULL;
	pp = exe;
	if (len)
	  {
	     strcpy(exe, homedir);
	     pp += len;
	     if (*(pp - 1) != '/')
	       {
		  *pp = '/';
		  pp++;
	       }
	  }
     }
   else
     {
	if (exe) free(exe);
	exe = malloc(exe2 - exe1 + 1);
	if (!exe) return NULL;
	pp = exe;
     }
   p = exe1;
   restart = 0;
   in_quot_dbl = 0;
   in_quot_sing = 0;
   while (*p)
     {
	if (in_quot_sing)
	  {
	     if (*p == '\'')
	       in_quot_sing = 0;
	     else
	       {
		  *pp = *p;
		  pp++;
	       }
	  }
	else if (in_quot_dbl)
	  {
	     if (*p == '\"')
	       in_quot_dbl = 0;
	     else
	       {
		  /* techcincally this is wrong. double quotes also accept
		   * special chars:
		   *
		   * $, `, \
		   */
		  *pp = *p;
		  pp++;
	       }
	  }
	else
	  {
	     /* technically we should handle special chars:
	      *
	      * $, `, \, etc.
	      */
	     if ((p > exe1) && (p[-1] == '\\'))
	       {
		  if (*p != '\n')
		    {
		       *pp = *p;
		       pp++;
		    }
	       }
	     else if ((p > exe1) && (*p == '='))
	       {
		  restart = 1;
		  *pp = *p;
		  pp++;
	       }
	     else if (*p == '\'')
	       in_quot_sing = 1;
	     else if (*p == '\"')
	       in_quot_dbl = 1;
	     else if (isspace(*p))
	       {
		  if (restart)
		    goto restart;
		  else
		    break;
	       }
	     else
	       {
		  *pp = *p;
		  pp++;
	       }
	  }
	p++;
     }
   *pp = 0;
   return exe;
}

EAPI char *
ecore_file_strip_ext(const char *path)
{
   char *p, *file = NULL;

   p = strrchr(path, '.');
   if (!p)
     {
	file = strdup(path);
     }
   else if (p != path)
     {
	file = malloc(((p - path) + 1) * sizeof(char));
	if (file)
	  {
	     memcpy(file, path, (p - path));
	     file[p - path] = 0;
	  }
     }

   return file;
}
