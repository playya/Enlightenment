#include "util.h"

time_t
e_file_modified_time(char *file)
{
   struct stat         st;
   
   if (stat(file, &st) < 0) return 0;
   return st.st_mtime;
}

void
e_set_env(char *variable, char *content)
{
   char env[PATH_MAX];
   
   sprintf(env, "%s=%s", variable, content);
   putenv(env);
}

int
e_file_exists(char *file)
{
   struct stat         st;
   
   if (stat(file, &st) < 0) return 0;
   return 1;
}

int
e_file_is_dir(char *file)
{
   struct stat         st;
   
   if (stat(file, &st) < 0) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

char *
e_file_home(void)
{
   static char *home = NULL;
   
   if (home) return home;
   home = getenv("HOME");
   if (!home) home = getenv("TMPDIR");
   if (!home) home = "/tmp";
   return home;
}

static mode_t       default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

int
e_file_mkdir(char *dir)
{
   if (mkdir(dir, default_mode) < 0) return 0;
   return 1;
}

int
e_file_cp(char *src, char *dst)
{
   FILE *f1, *f2;
   char buf[16384];
   size_t num;
   
   f1 = fopen(src, "rb");
   if (!f1) return 0;
   f2 = fopen(dst, "wb");
   if (!f2)
     {
	fclose(f1);
	return 0;
     }
   while ((num = fread(buf, 1, 16384, f1)) > 0) fwrite(buf, 1, num, f2);
   fclose(f1);
   fclose(f2);
   return 1;
}

char *
e_file_real(char *file)
{
   char buf[PATH_MAX];
   char *f;
   
   if (!realpath(file, buf)) return strdup("");
   e_strdup(f, buf);
   return f;
}

char *
e_file_get_file(char *file)
{
   char *p;
   char *f;
   
   p = strrchr(file, '/');
   if (!p) 
     {
	e_strdup(f, file);
	return f;
     }
   e_strdup(f, &(p[1]));
   return f;
}

char *
e_file_get_dir(char *file)
{
   char *p;
   char *f;
   char buf[PATH_MAX];
   
   strcpy(buf, file);
   p = strrchr(buf, '/');
   if (!p) 
     {
	e_strdup(f, file);
	return f;
     }
   *p = 0;
   e_strdup(f, buf);
   return f;
}

void *
e_memdup(void *data, int size)
{
   void *data_dup;
   
   data_dup = malloc(size);
   if (!data_dup) return NULL;
   memcpy(data_dup, data, size);
   return data_dup;
}

int
e_glob_matches(char *str, char *glob)
{
   if (!fnmatch(glob, str, 0)) return 1;
   return 0;
}

int
e_file_can_exec(struct stat *st)
{
   static int have_uid = 0;
   static uid_t uid = -1;
   static gid_t gid = -1;
   int ok;
   
   if (!st) return 0;
   ok = 0;
   if (!have_uid) uid = getuid();
   if (!have_uid) gid = getgid();
   have_uid = 1;
   if (st->st_uid == uid)
     {
	if (st->st_mode & S_IXUSR) ok = 1;
     }
   else if (st->st_gid == gid)
     {
	if (st->st_mode & S_IXGRP) ok = 1;
     }
   else
     {
	if (st->st_mode & S_IXOTH) ok = 1;
     }
   return ok;
}

char *
e_file_link(char *link)
{
   char buf[PATH_MAX];
   char *f;
   int count;
   
   if ((count = readlink(link, buf, sizeof(buf))) < 0) return NULL;
   buf[count] = 0;
   e_strdup(f, buf);
   return f;
}

Evas_List 
e_file_list_dir(char *dir)
{
   DIR                *dirp;
   struct dirent      *dp;
   Evas_List           list;
   
   dirp = opendir(dir);
   if (!dirp) return NULL;
   list = NULL;
   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) &&
	    (strcmp(dp->d_name, "..")))
	  {
	     Evas_List l;
	     char *f;
	     
	     /* insertion sort */
	     for (l = list; l; l = l->next)
	       {
		  if (strcmp(l->data, dp->d_name) > 0)
		    {
		       e_strdup(f, dp->d_name);
		       list = evas_list_prepend_relative(list, f, l->data);
		       break;
		    }
	       }
	     /* nowhwre to go? just append it */
	     e_strdup(f, dp->d_name);
	     if (!l) list = evas_list_append(list, f);
	  }
     }
   closedir(dirp);
   return list;
}
