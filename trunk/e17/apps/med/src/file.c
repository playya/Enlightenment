#include "e.h"
#include "debug.h"
#include "file.h"
#include "util.h"

time_t
e_file_mod_time(char *file)
{
   struct stat         st;

   D_ENTER;
   
   if (stat(file, &st) < 0) D_RETURN_(0);

   D_RETURN_(st.st_mtime);
}

int
e_file_exists(char *file)
{
   struct stat         st;
   
   D_ENTER;
   
   if (stat(file, &st) < 0) D_RETURN_(0);

   D_RETURN_(1);
}

int
e_file_is_dir(char *file)
{
   struct stat         st;
   
   D_ENTER;
   
   if (stat(file, &st) < 0) D_RETURN_(0);
   if (S_ISDIR(st.st_mode)) D_RETURN_(1);

   D_RETURN_(0);
}

static mode_t       default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

int
e_file_mkdir(char *dir)
{
   D_ENTER;
   
   if (mkdir(dir, default_mode) < 0) D_RETURN_(0);

   D_RETURN_(1);
}

int
e_file_cp(char *src, char *dst)
{
   FILE *f1, *f2;
   char buf[16384];
   size_t num;
   
   D_ENTER;
   
   f1 = fopen(src, "rb");
   if (!f1) D_RETURN_(0);
   f2 = fopen(dst, "wb");
   if (!f2)
     {
	fclose(f1);
	D_RETURN_(0);
     }
   while ((num = fread(buf, 1, 16384, f1)) > 0) fwrite(buf, 1, num, f2);
   fclose(f1);
   fclose(f2);

   D_RETURN_(1);
}

char *
e_file_realpath(char *file)
{
   char buf[PATH_MAX];
   char *f;
   
   D_ENTER;
   
   if (!realpath(file, buf)) D_RETURN_(strdup(""));
   e_strdup(f, buf);

   D_RETURN_(f);
}

char *
e_file_get_file(char *path)
{
  char *result = NULL;

  D_ENTER;

  if (!path)
    D_RETURN_(NULL);
  
  if ((result = strrchr(path, '/')))
    result++;
  else
    result = path;
  
  D_RETURN_(result);
}


char *
e_file_get_dir(char *file)
{
   char *p;
   char *f;
   char buf[PATH_MAX];
   
   D_ENTER;
   
   strcpy(buf, file);
   p = strrchr(buf, '/');
   if (!p) 
     {
	e_strdup(f, file);
	D_RETURN_(f);
     }
   *p = 0;
   e_strdup(f, buf);

   D_RETURN_(f);
}

int
e_file_can_exec(struct stat *st)
{
   static int have_uid = 0;
   static uid_t uid = -1;
   static gid_t gid = -1;
   int ok;
   
   D_ENTER;
   
   if (!st) D_RETURN_(0);
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

   D_RETURN_(ok);
}

char *
e_file_readlink(char *link)
{
   char buf[PATH_MAX];
   char *f;
   int count;
   
   D_ENTER;
   
   if ((count = readlink(link, buf, sizeof(buf))) < 0) D_RETURN_(NULL);
   buf[count] = 0;
   e_strdup(f, buf);

   D_RETURN_(f);
}

Evas_List * 
e_file_ls(char *dir)
{
   DIR                *dirp;
   struct dirent      *dp;
   Evas_List *           list;
   
   D_ENTER;
   
   dirp = opendir(dir);
   if (!dirp) D_RETURN_(NULL);
   list = NULL;
   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) &&
	    (strcmp(dp->d_name, "..")))
	  {
	     Evas_List * l;
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

   D_RETURN_(list);
}
