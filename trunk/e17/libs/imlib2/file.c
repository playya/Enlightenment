#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "file.h"

static void __imlib_FileFieldWord(char *s, int num, char *wd);

char               *
__imlib_FileExtension(char *file)
{
  char               *p;

  p = strrchr(file, '.');
  if (p != NULL)
    return(p + 1);
  return("");
}

int
__imlib_FileExists(char *s)
{
  struct stat         st;

  if ((!s) || (!*s))
    return(0);
  if (stat(s, &st) < 0)
    return(0);
  return(1);
}

int
__imlib_FileIsFile(char *s)
{
  struct stat         st;

  if ((!s) || (!*s))
    return(0);
  if (stat(s, &st) < 0)
    return(0);
  if (S_ISREG(st.st_mode))
    return(1);
  return(0);
}

int
__imlib_FileIsDir(char *s)
{
  struct stat         st;

  if ((!s) || (!*s))
    return(0);
  if (stat(s, &st) < 0)
    return(0);
  if (S_ISDIR(st.st_mode))
    return(1);
  return(0);
}

char              **
__imlib_FileDir(char *dir, int *num)
{
  int                 i, dirlen;
  int                 done = 0;
  DIR                *dirp;
  char              **names;
  struct dirent      *dp;

  if ((!dir) || (!*dir))
    return(0);
  dirp = opendir(dir);
  if (!dirp)
    {
      *num = 0;
      return(NULL);
    }
  /* count # of entries in dir (worst case) */
  for (dirlen = 0; (dp = readdir(dirp)) != NULL; dirlen++);
  if (!dirlen)
    {
      closedir(dirp);
      *num = dirlen;
      return(NULL);
    }
  names = (char **)malloc(dirlen * sizeof(char *));

  if (!names)
    return(NULL);

  rewinddir(dirp);
  for (i = 0; i < dirlen;)
    {
      dp = readdir(dirp);
      if (!dp)
	break;
      if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	{
	  names[i] = strdup(dp->d_name);
	  i++;
	}
    }

  if (i < dirlen)
    dirlen = i;			/* dir got shorter... */
  closedir(dirp);
  *num = dirlen;
  /* do a simple bubble sort here to alphanumberic it */
  while (!done)
    {
      done = 1;
      for (i = 0; i < dirlen - 1; i++)
	{
	  if (strcmp(names[i], names[i + 1]) > 0)
	    {
	      char               *tmp;

	      tmp = names[i];
	      names[i] = names[i + 1];
	      names[i + 1] = tmp;
	      done = 0;
	    }
	}
    }
  return(names);
}

void
__imlib_FileFreeDirList(char **l, int num)
{
  if (!l)
    return;
  while (num--)
    if (l[num])
      free(l[num]);
  free(l);
  return;
}

void
__imlib_FileDel(char *s)
{
  if ((!s) || (!*s))
    return;
  unlink(s);
  return;
}

time_t
__imlib_FileModDate(char *s)
{
  struct stat         st;

  if ((!s) || (!*s))
    return(0);
  if (!stat(s, &st) < 0)
    return(0);
  if (st.st_mtime > st.st_ctime)
    {
      return(st.st_mtime);
    }
  else
    return(st.st_ctime);
  return(0);
}

char               *
__imlib_FileHomeDir(int uid)
{
  static int          usr_uid = -1;
  static char        *usr_s = NULL;
  char               *s;
  struct passwd      *pwd;

  s = getenv("HOME");
  if (s)
    return strdup(s);
  if (usr_uid < 0)
    usr_uid = getuid();
  if ((uid == usr_uid) && (usr_s))
    {
      return(strdup(usr_s));
    }
  pwd = getpwuid(uid);
  if (pwd)
    {
      s = strdup(pwd->pw_dir);
      if (uid == usr_uid)
	usr_s = strdup(s);
      return(s);
    }
  return NULL;
}

/* gets word number [num] in the string [s] and copies it into [wd] */
/* wd is NULL terminated. If word [num] does not exist wd = "" */
/* NB: this function now handles quotes so for a line: */
/* Hello to "Welcome sir - may I Help" Shub Foo */
/* Word 1 = Hello */
/* Word 2 = to */
/* Word 3 = Welcome sir - may I Help */
/* Word 4 = Shub */
/* word 5 = Foo */

char               *
__imlib_FileField(char *s, int field)
{
  char                buf[4096];

  buf[0] = 0;
  __imlib_FileFieldWord(s, field + 1, buf);
  if (buf[0])
    {
      if ((!strcmp(buf, "NULL")) ||
	  (!strcmp(buf, "(null)")))
	return(NULL);
      return(strdup(buf));
    }
  return(NULL);
}










static void
__imlib_FileFieldWord(char *s, int num, char *wd)
{
  char               *cur, *start, *end;
  int                 count, inword, inquote, len;

  if (!s)
    return;
  if (!wd)
    return;
  *wd = 0;
  if (num <= 0)
    return;
  cur = s;
  count = 0;
  inword = 0;
  inquote = 0;
  start = NULL;
  end = NULL;
  while ((*cur) && (count < num))
    {
      if (inword)
	{
	  if (inquote)
	    {
	      if (*cur == '"')
		{
		  inquote = 0;
		  inword = 0;
		  end = cur;
		  count++;
		}
	    }
	  else
	    {
	      if (isspace(*cur))
		{
		  end = cur;
		  inword = 0;
		  count++;
		}
	    }
	}
      else
	{
	  if (!isspace(*cur))
	    {
	      if (*cur == '"')
		{
		  inquote = 1;
		  start = cur + 1;
		}
	      else
		start = cur;
	      inword = 1;
	    }
	}
      if (count == num)
	break;
      cur++;
    }
  if (!start)
    return;
  if (!end)
    end = cur;
  if (end <= start)
    return;
  len = (int)(end - start);
  if (len > 4000)
    len = 4000;
  if (len > 0)
    {
      strncpy(wd, start, len);
      wd[len] = 0;
    }
  return;
}

