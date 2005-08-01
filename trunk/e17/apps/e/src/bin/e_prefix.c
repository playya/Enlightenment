/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static int _e_prefix_fallbacks(void);
static int _e_prefix_try_proc(void);
static int _e_prefix_try_argv(char *argv0);

/* local subsystem globals */
static char *_exe_path = NULL;
static char *_prefix_path = NULL;
static char *_prefix_path_locale = NULL;
static char *_prefix_path_bin = NULL;
static char *_prefix_path_data = NULL;
static char *_prefix_path_lib = NULL;

/* externally accessible functions */
int
e_prefix_determine(char *argv0)
{
   char *p;

   IF_FREE(_exe_path);
   IF_FREE(_prefix_path);
   IF_FREE(_prefix_path_locale);
   IF_FREE(_prefix_path_bin);
   IF_FREE(_prefix_path_data);
   IF_FREE(_prefix_path_lib);
   
   if (!_e_prefix_try_proc())
     {
	if (!_e_prefix_try_argv(argv0))
	  {
	     _e_prefix_fallbacks();
	     return 0;
	  }
     }
   /* _exe_path is now a full absolute path TO this exe - figure out rest */
   /*   if
    * exe        = /blah/whatever/bin/exe
    *   then
    * prefix     = /blah/whatever
    * bin_dir    = /blah/whatever/bin
    * data_dir   = /blah/whatever/share/enlightenment
    * locale_dir = /blah/whatever/share/locale
    * lib_dir    = /blah/whatever/lib
    */
   p = strrchr(_exe_path, '/');
   if (p)
     {
	p--;
	while (p >= _exe_path)
	  {
	     if (*p == '/')
	       {
		  _prefix_path = malloc(p - _exe_path + 1);
		  if (_prefix_path)
		    {
		       strncpy(_prefix_path, _exe_path, p - _exe_path);
		       _prefix_path[p - _exe_path] = 0;
		       
		       _prefix_path_locale = malloc(strlen(_prefix_path) + 1 +
						    strlen("/share/locale"));
		       strcpy(_prefix_path_locale, _prefix_path);
		       strcat(_prefix_path_locale, "/share/locale");
		       
		       _prefix_path_bin = malloc(strlen(_prefix_path) + 1 +
						    strlen("/bin"));
		       strcpy(_prefix_path_bin, _prefix_path);
		       strcat(_prefix_path_bin, "/bin");
		       
		       _prefix_path_data = malloc(strlen(_prefix_path) + 1 +
						    strlen("/share/enlightenment"));
		       strcpy(_prefix_path_data, _prefix_path);
		       strcat(_prefix_path_data, "/share/enlightenment");

		       _prefix_path_lib = malloc(strlen(_prefix_path) + 1 +
						    strlen("/lib"));
		       strcpy(_prefix_path_lib, _prefix_path);
		       strcat(_prefix_path_lib, "/lib");
		       
		       printf("DYNAMIC DETERMINED PREFIX: %s\n", _prefix_path);
		       return 1;
		    }
		  else
		    {
		       free(_exe_path);
		       _exe_path = NULL;
		       _e_prefix_fallbacks();
		       return 0;
		    }
	       }
	     p--;
	  }
     }
   free(_exe_path);
   _exe_path = NULL;
   _e_prefix_fallbacks();
   return 0;
}

void
e_prefix_fallback(void)
{
   IF_FREE(_exe_path);
   IF_FREE(_prefix_path);
   IF_FREE(_prefix_path_locale);
   IF_FREE(_prefix_path_bin);
   IF_FREE(_prefix_path_data);
   IF_FREE(_prefix_path_lib);
   _e_prefix_fallbacks();
}

const char *
e_prefix_get(void)
{
   return _prefix_path;
}

const char *
e_prefix_locale_get(void)
{
   return _prefix_path_locale;
}

const char *
e_prefix_bin_get(void)
{
   return _prefix_path_bin;
}

const char *
e_prefix_data_get(void)
{
   return _prefix_path_data;
}

const char *
e_prefix_lib_get(void)
{
   return _prefix_path_lib;
}

/* local subsystem functions */
static int
_e_prefix_fallbacks(void)
{
   char *p;

   _prefix_path = strdup(PACKAGE_BIN_DIR);
   p = strrchr(_prefix_path, '/');
   if (p) *p = 0;
   _prefix_path_locale = strdup(LOCALE_DIR);
   _prefix_path_bin = strdup(PACKAGE_BIN_DIR);
   _prefix_path_data = strdup(PACKAGE_DATA_DIR);
   _prefix_path_lib = strdup(PACKAGE_LIB_DIR);
   printf("WARNING: Enlightenment could not determine its installed prefix\n"
	  "         and is falling back on the compiled in default:\n"
	  "         %s\n", _prefix_path);
}

static int
_e_prefix_try_proc(void)
{
   FILE *f;
   char buf[4096];
   void *func = NULL;

   func = (void *)_e_prefix_try_proc;
   f = fopen("/proc/self/maps", "r");
   if (!f) return 0;
   while (fgets(buf, sizeof(buf), f))
     {
	int len;
	char *p, mode[5] = "";
	unsigned long ptr1 = 0, ptr2 = 0;
	
	len = strlen(buf);
	if (buf[len - 1] == '\n')
	  {
	     buf[len - 1] = 0;
	     len--;
	  }
	if (sscanf(buf, "%lx-%lx %4s", &ptr1, &ptr2, mode) == 3)
	  {
	     if (!strcmp(mode, "r-xp"))
	       {
		  if (((void *)ptr1 <= func) && (func < (void *)ptr2))
		    {
		       p = strchr(buf, '/');
		       if (p)
			 {
			    if (len > 10)
			      {
				 if (!strcmp(buf + len - 10, " (deleted)"))
				   buf[len - 10] = 0;
			      }
			    _exe_path = strdup(p);
			    fclose(f);
			    return 1;
			 }
		       else
			 break;
		    }
	       }
	  }
     }
   fclose(f);
   return 0;
}

static int
_e_prefix_try_argv(char *argv0)
{
   char *path, *p, *cp, *s;
   int len, lenexe;
#ifdef PATH_MAX
   char buf[PATH_MAX], buf2[PATH_MAX], buf3[PATH_MAX];
#else
   char buf[4096], buf2[4096], buf3[4096];
#endif	
   
   /* 1. is argv0 abs path? */
   if (argv0[0] == '/')
     {
	_exe_path = strdup(argv0);
	if (access(_exe_path, X_OK) == 0) return 1;
	free(_exe_path);
	_exe_path = NULL;
	return 0;
     }
   /* 2. relative path */
   if (strchr(argv0, '/'))
     {
	if (getcwd(buf3, sizeof(buf3)))
	  {
	     snprintf(buf2, sizeof(buf2), "%s/%s", buf3, argv0);
	     if (realpath(buf2, buf))
	       {
		  _exe_path = strdup(buf);
		  if (access(_exe_path, X_OK) == 0) return 1;
		  free(_exe_path);
		  _exe_path = NULL;
	       }
	  }
     }
   /* 3. argv0 no path - look in PATH */
   path = getenv("PATH");
   if (!path) return 0;
   p = path;
   cp = p;
   lenexe = strlen(argv0);
   while ((p = strchr(cp, ':')))
     {
	len = p - cp;
	s = malloc(len + 1 + lenexe + 1);
	if (s)
	  {
	     strncpy(s, cp, len);
	     s[len] = '/';
	     strcpy(s + len + 1, argv0);
	     if (realpath(s, buf))
	       {
		  if (access(buf, X_OK) == 0)
		    {
		       _exe_path = strdup(buf);
		       free(s);
		       return 1;
		    }
	       }
	     free(s);
	  }
        cp = p + 1;
     }
   len = strlen(cp);
   s = malloc(len + 1 + lenexe + 1);
   if (s)
     {
	strncpy(s, cp, len);
	s[len] = '/';
	strcpy(s + len + 1, argv0);
	if (realpath(s, buf))
	  {
	     if (access(buf, X_OK) == 0)
	       {
		  _exe_path = strdup(buf);
		  free(s);
		  return 1;
	       }
	  }
	free(s);
     }
   /* 4. big problems. arg[0] != executable - weird execution */
   return 0;
}
